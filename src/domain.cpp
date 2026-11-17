#include "domain.hpp"
#include "json.hpp"

#include <QRegularExpression>
#include <QTime>
#include <QTimeZone>
#include <QUrl>

#include <limits>
#include <stdexcept>

namespace {
using Json = nlohmann::json;

QString String(const Json& object, const char* field) {
    const auto it = object.find(field);
    if (it == object.end() || it->is_null()) return {};
    if (!it->is_string()) throw std::runtime_error(std::string(field) + ": expected string");
    return QString::fromStdString(it->get<std::string>());
}

std::optional<std::int64_t> Count(const Json& object, const char* field) {
    const auto it = object.find(field);
    if (it == object.end() || it->is_null()) return std::nullopt;
    if (!it->is_number_integer() ||
        (it->is_number_unsigned() && it->get<std::uint64_t>() > (std::numeric_limits<std::int64_t>::max)()))
        throw std::runtime_error(std::string(field) + ": expected nonnegative int64");
    const auto value = it->get<std::int64_t>();
    if (value < 0) throw std::runtime_error(std::string(field) + ": negative count");
    return value;
}

const Json& Array(const Json& object, const char* field) {
    static const Json empty = Json::array();
    const auto it = object.find(field);
    if (it == object.end() || it->is_null()) return empty;
    if (!it->is_array()) throw std::runtime_error(std::string(field) + ": expected array");
    return *it;
}

void Object(const Json& value) {
    if (!value.is_object()) throw std::runtime_error("Expected JSON object");
}

WarPhase Phase(QString state) {
    state = state.toLower().remove('_');
    if (state == "notinwar") return WarPhase::None;
    if (state == "preparation") return WarPhase::Preparation;
    if (state == "war" || state == "inwar") return WarPhase::Active;
    if (state == "ended" || state == "warended") return WarPhase::Ended;
    if (state == "inmatchmaking" || state == "enterwar" || state == "matched") return WarPhase::Waiting;
    return WarPhase::Unknown;
}

QDateTime Time(const Json& object, const char* field) {
    const auto text = String(object, field);
    if (text.isEmpty()) return {};
    auto time = QDateTime::fromString(text, "yyyyMMdd'T'HHmmss.zzz'Z'");
    if (time.isValid()) time.setTimeZone(QTimeZone::UTC);
    else time = QDateTime::fromString(text, Qt::ISODate);
    if (!time.isValid()) throw std::runtime_error(std::string(field) + ": invalid timestamp");
    return time.toUTC();
}

WarSide Side(const Json& object) {
    Object(object);
    WarSide side;
    side.tag = String(object, "tag");
    side.roster_known = object.contains("members") && object["members"].is_array();
    for (const auto& member : Array(object, "members")) {
        Object(member);
        const auto tag = String(member, "tag");
        if (tag.isEmpty() || side.members.contains(tag)) throw std::runtime_error("Invalid or duplicate war member");
        std::optional<std::int64_t> count = std::nullopt;
        if (member.contains("attacks") && member["attacks"].is_array()) {
            QSet<std::int64_t> orders;
            for (const auto& attack : member["attacks"]) {
                Object(attack);
                const auto order = Count(attack, "order");
                if (!order || orders.contains(*order) || String(attack, "attackerTag") != tag)
                    throw std::runtime_error("Invalid or duplicate war attack");
                orders.insert(*order);
            }
            count = static_cast<std::int64_t>(orders.size());
        } else if (member.contains("attacks") && !member["attacks"].is_null()) {
            throw std::runtime_error("attacks: expected array");
        }
        // shortcut: omitted attacks stay unknown until omission semantics are verified against the live API.
        side.members.insert(tag, count);
    }
    return side;
}

War ParseWar(const Json& object) {
    War war;
    war.phase = Phase(String(object, "state"));
    war.limit = Count(object, "attacksPerMember");
    war.start = Time(object, "startTime");
    war.end = Time(object, "endTime");
    if (object.contains("clan")) war.clan = Side(object.at("clan"));
    if (object.contains("opponent")) war.opponent = Side(object.at("opponent"));
    if (war.start.isValid() && war.end.isValid() && war.end <= war.start)
        throw std::runtime_error("War ends before it starts");
    return war;
}
}

QString NormalizeTag(QString tag) {
    tag = tag.trimmed().toUpper();
    static const QRegularExpression pattern("^#[A-Z0-9]{1,63}$");
    if (!pattern.match(tag).hasMatch()) throw std::invalid_argument("Тег должен начинаться с # и содержать только латинские буквы и цифры");
    return tag;
}

QString EncodedTag(const QString& tag) { return QString::fromLatin1(QUrl::toPercentEncoding(tag)); }
QString PlayerKey(const QString& tag) { return "/players/" + EncodedTag(tag); }
QString WarKey(const QString& clan) { return "/clans/" + EncodedTag(clan) + "/currentwar"; }
QString CwlKey(const QString& clan) { return WarKey(clan) + "/leaguegroup"; }

ApiData ParseResource(ResourceKind kind, const QByteArray& body) {
    if (kind == ResourceKind::Player) return ParsePlayerProfile(std::string_view(body.constData(), static_cast<std::size_t>(body.size())));
    const auto object = Json::parse(body.constData(), body.constData() + body.size());
    Object(object);
    if (kind == ResourceKind::War || kind == ResourceKind::CwlWar) return ParseWar(object);
    if (kind == ResourceKind::CwlGroup) {
        CwlGroup group;
        group.season = String(object, "season");
        group.phase = Phase(String(object, "state"));
        for (const auto& clan : Array(object, "clans")) {
            Object(clan);
            const auto tag = String(clan, "tag");
            if (tag.isEmpty() || group.rosters.contains(tag) || !clan.contains("members") || !clan["members"].is_array())
                throw std::runtime_error("Invalid CWL clan roster");
            QSet<QString> members;
            for (const auto& member : Array(clan, "members")) {
                Object(member);
                const auto player = String(member, "tag");
                if (player.isEmpty() || members.contains(player)) throw std::runtime_error("Invalid CWL member");
                members.insert(player);
            }
            group.rosters.insert(tag, members);
        }
        for (const auto& round : Array(object, "rounds")) {
            Object(round);
            QStringList tags;
            for (const auto& value : Array(round, "warTags")) {
                if (!value.is_string()) throw std::runtime_error("Invalid CWL war tag");
                const auto tag = QString::fromStdString(value.get<std::string>());
                if (tag != "#0") tags.append(NormalizeTag(tag));
            }
            group.rounds.append(tags);
        }
        return group;
    }
    RankedGroup group;
    if (const auto it = object.find("parameters"); it != object.end() && !it->is_null()) {
        Object(*it);
        group.limit = Count(*it, "maxBattles");
        if (const auto season = it->find("seasonId"); season != it->end() && !season->is_null()) {
            if (season->is_string()) group.period = String(*it, "seasonId");
            else group.period = QString::number(*Count(*it, "seasonId"));
        }
    }
    for (const auto& member : Array(object, "members")) {
        Object(member);
        const auto tag = String(member, "playerTag");
        if (tag.isEmpty() || group.members.contains(tag)) throw std::runtime_error("Invalid or duplicate Ranked member");
        const auto wins = Count(member, "attackWinCount");
        const auto losses = Count(member, "attackLoseCount");
        std::optional<std::int64_t> used = std::nullopt;
        if (wins && losses) {
            if (*wins > (std::numeric_limits<std::int64_t>::max)() - *losses)
                throw std::runtime_error("Ranked attack count overflow");
            used = *wins + *losses;
        }
        group.members.insert(tag, used);
    }
    return group;
}

Activity WarActivity(const War& war, const QString& player, const QDateTime& now) {
    Activity result;
    result.end = war.end;
    if (war.phase == WarPhase::None) { result.state = ActivityState::NotParticipating; result.detail = "Клан не воюет"; return result; }
    if (war.phase == WarPhase::Unknown) { result.state = ActivityState::InsufficientData; return result; }
    if (war.phase == WarPhase::Waiting) { result.state = ActivityState::Waiting; result.detail = "Поиск соперника"; return result; }
    const WarSide* side = nullptr;
    if (war.clan.members.contains(player)) side = &war.clan;
    else if (war.opponent.members.contains(player)) side = &war.opponent;
    if (!side) {
        result.state = war.clan.roster_known && war.opponent.roster_known ? ActivityState::NotParticipating : ActivityState::InsufficientData;
        return result;
    }
    result.used = side->members.value(player);
    result.limit = war.limit;
    if (result.used && result.limit && *result.used > *result.limit) {
        result.used.reset(); result.limit.reset();
    }
    if (war.phase == WarPhase::Ended) { result.state = ActivityState::Ended; return result; }
    if (war.phase == WarPhase::Preparation) { result.state = ActivityState::Waiting; result.detail = "Подготовка"; return result; }
    if (war.start.isValid() && war.start > now) {
        result.state = ActivityState::Waiting; result.detail = "Ожидается начало войны"; return result;
    }
    if (war.end.isValid() && war.end <= now) {
        result.state = ActivityState::Waiting; result.detail = "Срок истёк; требуется обновление"; return result;
    }
    if (!result.used || !result.limit) { result.state = ActivityState::InsufficientData; return result; }
    result.attention = *result.used < *result.limit;
    result.state = result.attention ? ActivityState::AttacksRemaining : ActivityState::Done;
    result.detail = QString("Осталось %1 из %2").arg(*result.limit - *result.used).arg(*result.limit);
    return result;
}

Activity CwlActivity(const CwlGroup& group, const QVector<RoundWar>& wars,
                     const QString& clan, const QString& player, const QDateTime& now) {
    if (group.phase == WarPhase::None) return {.state = ActivityState::NotParticipating};
    if (group.phase == WarPhase::Unknown) return {.state = ActivityState::InsufficientData};
    if (group.season.isEmpty() || !group.rosters.contains(clan)) return {.state = ActivityState::InsufficientData};
    if (!group.rosters.value(clan).contains(player)) return {.state = ActivityState::NotParticipating, .detail = "Нет в сезонной заявке"};
    if (group.phase == WarPhase::Ended) return {.state = ActivityState::Ended, .detail = group.season};
    const RoundWar* selected = nullptr;
    auto priority = [&now](const War& war) {
        if (war.phase == WarPhase::Active) return war.end.isValid() && war.end <= now ? 1 : 3;
        if (war.phase == WarPhase::Preparation) return 2;
        if (war.phase == WarPhase::Ended) return 1;
        return 0;
    };
    for (const auto& item : wars) {
        if (item.war.clan.tag != clan && item.war.opponent.tag != clan) continue;
        if (!selected || priority(item.war) > priority(selected->war) ||
            (priority(item.war) == priority(selected->war) && item.round > selected->round)) selected = &item;
    }
    if (!selected) return {.state = ActivityState::Waiting, .detail = "Раунд ещё не опубликован"};
    auto result = WarActivity(selected->war, player, now);
    const auto heading = QString("%1 · раунд %2").arg(group.season).arg(selected->round);
    result.detail = heading + (result.detail.isEmpty() ? QString{} : " · " + result.detail);
    return result;
}

Activity RankedActivity(const RankedGroup& group, const PlayerProfile& profile, const QDateTime& now) {
    Activity result;
    result.detail = "Нет полных данных текущего турнира Ranked";
    if (!now.isValid() || !profile.league_tier || !profile.league_tier->id || !profile.current_league_season_id) return result;
    const auto id = *profile.league_tier->id;
    if (id == 105000036) { result.detail = "Суточный учёт Legend I пока недоступен"; return result; }
    if (id < 105000001 || id > 105000035) { result.detail = "Неизвестные правила лиги Ranked"; return result; }
    const auto tier = id - 105000000;
    auto start = QDateTime(now.toUTC().date().addDays(1 - now.toUTC().date().dayOfWeek()), QTime(5, 0), QTimeZone::UTC);
    if (start > now) start = start.addDays(-7);
    const auto expected = QString::number(start.toSecsSinceEpoch());
    const auto& season = *profile.current_league_season_id;
    const auto period = season.is_string() ? QString::fromStdString(season.get<std::string>()) : QString::fromStdString(season.dump());
    if (period != expected || (!group.period.isEmpty() && group.period != period)) {
        result.detail = "Период Ranked изменился; требуется обновление"; return result;
    }
    result.end = start.addDays(7);
    const auto member = group.members.constFind(QString::fromStdString(profile.tag));
    if (member == group.members.cend() || !member.value()) return result;
    // shortcut: API may omit maxBattles; update the table when rules change (source: docs/api_contract.md).
    const auto fallback = tier <= 6 ? 6 : tier <= 12 ? 8 : tier <= 18 ? 10 : tier <= 27 ? 12
                        : tier <= 30 ? 14 : tier <= 33 ? 18 : tier == 34 ? 24 : 30;
    const auto limit = group.limit.value_or(fallback);
    if (limit <= 0 || *member.value() > limit) { result.detail = "Счётчики Ranked не соответствуют лимиту"; return result; }
    result.used = member.value();
    result.limit = limit;
    result.attention = *result.used < limit;
    result.state = result.attention ? ActivityState::AttacksRemaining : ActivityState::Done;
    result.detail = QString("Осталось %1 из %2").arg(limit - *result.used).arg(limit);
    if (!group.limit) result.detail += " · лимит по правилам лиги";
    return result;
}

bool SameWar(const War& a, const War& b) {
    return a.start.isValid() && a.start == b.start && !a.clan.tag.isEmpty() &&
        ((a.clan.tag == b.clan.tag && a.opponent.tag == b.opponent.tag) ||
         (a.clan.tag == b.opponent.tag && a.opponent.tag == b.clan.tag));
}

bool ResourceMatches(const QString& key, const ApiData& data) {
    if (const auto* player = std::get_if<PlayerProfile>(&data)) return key == PlayerKey(QString::fromStdString(player->tag));
    if (const auto* war = std::get_if<War>(&data); war && key.startsWith("/clans/") && !war->clan.tag.isEmpty())
        return key == WarKey(war->clan.tag) || key == WarKey(war->opponent.tag);
    return true;
}

QString ActivityText(ActivityState state) {
    switch (state) {
    case ActivityState::AttacksRemaining: return "Атаки остались";
    case ActivityState::Done: return "Выполнено";
    case ActivityState::NotParticipating: return "Не участвует";
    case ActivityState::Waiting: return "Ожидается";
    case ActivityState::Unavailable: return "Данные недоступны";
    case ActivityState::InsufficientData: return "Недостаточно данных";
    case ActivityState::Ended: return "Завершена";
    }
    return "Недостаточно данных";
}
