#include "accounts_model.hpp"

#include <QUrlQuery>

#include <exception>

namespace {
QString Text(const std::string& value) { return QString::fromStdString(value); }
QString ClanTag(const PlayerProfile* profile) {
    return profile && profile->clan && profile->clan->tag ? Text(*profile->clan->tag) : QString{};
}
QString CwlWarKey(const QString& tag) { return "/clanwarleagues/wars/" + EncodedTag(tag); }
QString RankedKey(const PlayerProfile& profile) {
    if (!profile.current_league_group_tag || !profile.current_league_season_id) return {};
    const auto& id = *profile.current_league_season_id;
    QString season;
    if (id.is_string()) season = Text(id.get<std::string>());
    else if (id.is_number_integer()) season = Text(id.dump());
    if (season.isEmpty() || profile.current_league_group_tag->empty()) return {};
    QUrlQuery query;
    query.addQueryItem("playerTag", Text(profile.tag));
    return "/leaguegroup/" + EncodedTag(Text(*profile.current_league_group_tag)) + "/" + EncodedTag(season)
        + "?" + query.toString(QUrl::FullyEncoded);
}
ResourceKind Kind(const QString& key) {
    if (key.startsWith("/players/")) return ResourceKind::Player;
    if (key.startsWith("/clanwarleagues/")) return ResourceKind::CwlWar;
    if (key.startsWith("/leaguegroup/")) return ResourceKind::RankedGroup;
    if (key.endsWith("/leaguegroup")) return ResourceKind::CwlGroup;
    return ResourceKind::War;
}
QString LocalTime(const QDateTime& at) { return at.toLocalTime().toString("dd.MM.yyyy HH:mm"); }
}

AccountsModel::AccountsModel(QString databasePath, QByteArray apiKey, QObject* parent)
    : QAbstractListModel(parent), database_path_(std::move(databasePath)), api_(std::move(apiKey)),
      store_(new LocalStore(database_path_)) {
    qRegisterMetaType<StoredState>();
    qRegisterMetaType<QList<AccountSettings>>();
    store_->moveToThread(&storage_thread_);
    connect(&storage_thread_, &QThread::started, store_, &LocalStore::load);
    connect(&storage_thread_, &QThread::finished, store_, &QObject::deleteLater);
    connect(store_, &LocalStore::loaded, this, [this](const StoredState& state, const QString& error) {
        if (!error.isEmpty()) { error_ = tr("Ошибка хранилища: %1").arg(error); emit stateChanged(); return; }
        beginResetModel();
        accounts_ = state.accounts;
        refresh_seconds_ = state.refresh_seconds;
        for (auto it = state.cache.cbegin(); it != state.cache.cend(); ++it) {
            try {
                Resource resource;
                resource.data = ParseResource(Kind(it.key()), it->body);
                if (!ResourceMatches(it.key(), *resource.data)) continue;
                resource.fetched = it->fetched;
                resources_.insert(it.key(), resource);
            } catch (const std::exception&) { /* Invalid cached data will be requested again. */ }
        }
        ready_ = true;
        endResetModel();
        if (refresh_seconds_) refresh_timer_.start(refresh_seconds_ * 1000);
        emit stateChanged();
        refresh();
    });
    connect(store_, &LocalStore::accountsSaved, this, [this](const QList<AccountSettings>& accounts, const QString& error) {
        saving_ = false;
        error_ = error;
        if (error.isEmpty()) {
            beginResetModel();
            accounts_ = accounts;
            endResetModel();
            emit accountsCommitted();
            refresh();
        }
        emit stateChanged();
    });
    connect(store_, &LocalStore::intervalSaved, this, [this](int seconds, const QString& error) {
        saving_ = false;
        error_ = error;
        if (error.isEmpty()) {
            refresh_seconds_ = seconds;
            refresh_timer_.stop();
            if (seconds) refresh_timer_.start(seconds * 1000);
            emit intervalCommitted();
        }
        notifyRows();
    });
    connect(store_, &LocalStore::failed, this, [this](const QString& error) { error_ = error; emit stateChanged(); });
    connect(&api_, &ApiClient::finished, this, &AccountsModel::received);
    connect(&refresh_timer_, &QTimer::timeout, this, [this] { if (!busy() && !saving_) refresh(); });
    connect(&clock_timer_, &QTimer::timeout, this, &AccountsModel::notifyRows);
    clock_timer_.start(30000);
    storage_thread_.start();
}

AccountsModel::~AccountsModel() {
    disconnect(&api_, nullptr, this, nullptr);
    api_.cancel();
    // Drain queued writes before closing the connection in its owning thread.
    QMetaObject::invokeMethod(store_, &LocalStore::close, Qt::BlockingQueuedConnection);
    storage_thread_.quit();
    storage_thread_.wait();
}

int AccountsModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : static_cast<int>(accounts_.size()); }
QVariant AccountsModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.column() != 0 || index.row() < 0 || index.row() >= accounts_.size()) return {};
    const auto& account = accounts_.at(index.row());
    if (role == Qt::UserRole + 1) return row(account);
    if (role == Qt::DisplayRole) return account.alias.isEmpty() ? account.tag : account.alias;
    return {};
}
QHash<int, QByteArray> AccountsModel::roleNames() const {
    static const QHash<int, QByteArray> roles = {{Qt::UserRole + 1, "account"}};
    return roles;
}
bool AccountsModel::ready() const { return ready_; }
bool AccountsModel::saving() const { return saving_; }
bool AccountsModel::busy() const { return pending_ > 0; }
int AccountsModel::count() const { return static_cast<int>(accounts_.size()); }
int AccountsModel::refreshMinutes() const { return refresh_seconds_ / 60; }
QString AccountsModel::databasePath() const { return database_path_; }
QString AccountsModel::error() const {
    if (!error_.isEmpty()) return error_;
    return api_.hasKey() ? QString{} : tr("Не задан API_KEY. Доступны локальные аккаунты и сохранённые данные.");
}

void AccountsModel::notifyRows() {
    if (!accounts_.isEmpty()) emit dataChanged(index(0), index(count() - 1), {Qt::UserRole + 1});
    emit stateChanged();
}

int AccountsModel::findAccount(const QString& tag) const {
    for (qsizetype i = 0; i < accounts_.size(); ++i) if (accounts_.at(i).tag == tag) return static_cast<int>(i);
    return -1;
}

void AccountsModel::saveAccounts(const QList<AccountSettings>& accounts) {
    saving_ = true;
    emit stateChanged();
    QMetaObject::invokeMethod(store_, [store = store_, accounts] { store->saveAccounts(accounts); }, Qt::QueuedConnection);
}

bool AccountsModel::addAccount(const QString& input) {
    if (!ready_ || saving_) return false;
    try {
        const auto tag = NormalizeTag(input);
        if (findAccount(tag) >= 0) throw std::invalid_argument("Этот аккаунт уже добавлен");
        auto accounts = accounts_;
        AccountSettings account;
        account.tag = tag;
        accounts.append(account);
        saveAccounts(accounts);
        return true;
    } catch (const std::exception& error) { error_ = QString::fromUtf8(error.what()); emit stateChanged(); return false; }
}

bool AccountsModel::updateAccount(const QString& tag, const QString& alias, const QString& warClan,
                                  const QString& cwlClan, const QString& transferClan, const QString& transferAt) {
    const auto position = findAccount(tag);
    if (!ready_ || saving_ || position < 0) return false;
    try {
        auto accounts = accounts_;
        auto& account = accounts[position];
        account.alias = alias.trimmed();
        account.war_clan = warClan.trimmed().isEmpty() ? QString{} : NormalizeTag(warClan);
        const auto cwl = cwlClan.trimmed().isEmpty() ? QString{} : NormalizeTag(cwlClan);
        if (account.cwl_clan != cwl) account.cwl_season.clear();
        account.cwl_clan = cwl;
        if (const auto* group = value<CwlGroup>(CwlKey(cwl))) account.cwl_season = group->season;
        account.transfer_clan = transferClan.trimmed().isEmpty() ? QString{} : NormalizeTag(transferClan);
        account.transfer_at = {};
        if (!transferAt.trimmed().isEmpty() && !account.transfer_clan.isEmpty()) {
            account.transfer_at = QDateTime::fromString(transferAt.trimmed(), Qt::ISODate);
            if (!account.transfer_at.isValid())
                throw std::invalid_argument("Укажите корректное время перехода: ГГГГ-ММ-ДДTчч:мм");
            account.transfer_at = account.transfer_at.toUTC();
        }
        saveAccounts(accounts);
        return true;
    } catch (const std::exception& error) { error_ = QString::fromUtf8(error.what()); emit stateChanged(); return false; }
}

void AccountsModel::removeAccount(const QString& tag) {
    const auto position = findAccount(tag);
    if (!ready_ || saving_ || position < 0) return;
    auto accounts = accounts_;
    accounts.removeAt(position);
    saveAccounts(accounts);
}

void AccountsModel::moveAccount(const QString& tag, int direction) {
    const auto position = findAccount(tag);
    if (!ready_ || saving_ || position < 0 || (direction != -1 && direction != 1)) return;
    const auto target = position + direction;
    if (target < 0 || target >= accounts_.size()) return;
    auto accounts = accounts_;
    accounts.swapItemsAt(position, target);
    saveAccounts(accounts);
}

void AccountsModel::setRefreshMinutes(int minutes) {
    if (!ready_ || saving_) return;
    if (minutes < 0 || minutes > 1440) { error_ = tr("Период: 0 (вручную) или 1–1440 минут"); emit stateChanged(); return; }
    saving_ = true;
    emit stateChanged();
    QMetaObject::invokeMethod(store_, [store = store_, minutes] { store->setInterval(minutes * 60); }, Qt::QueuedConnection);
}

QVariantMap AccountsModel::accountSettings(const QString& tag) const {
    const auto position = findAccount(tag);
    if (position < 0) return {};
    const auto& account = accounts_.at(position);
    return {{"tag", account.tag}, {"alias", account.alias}, {"warClan", account.war_clan}, {"cwlClan", account.cwl_clan},
            {"transferClan", account.transfer_clan}, {"transferAt", account.transfer_at.isValid()
                ? account.transfer_at.toLocalTime().toString("yyyy-MM-dd'T'HH:mm") : QString{}}};
}

void AccountsModel::refresh() {
    if (!ready_ || saving_) return;
    ++generation_;
    api_.cancel();
    pending_ = 0;
    requested_.clear();
    for (auto it = resources_.begin(); it != resources_.end(); ++it) it->loading = false;
    schedule();
    if (accounts_.isEmpty()) {
        resources_.clear();
        QMetaObject::invokeMethod(store_, [store = store_] { store->pruneCache({}); }, Qt::QueuedConnection);
    }
    notifyRows();
}

void AccountsModel::request(const QString& key, ResourceKind kind) {
    if (key.isEmpty() || requested_.contains(key)) return;
    requested_.insert(key);
    resources_[key].loading = true;
    ++pending_;
    api_.fetch(key, kind, generation_);
}

void AccountsModel::schedule() {
    for (const auto& account : std::as_const(accounts_)) {
        request(PlayerKey(account.tag), ResourceKind::Player);
        const auto* profile = value<PlayerProfile>(PlayerKey(account.tag));
        const auto current_clan = ClanTag(profile);
        const auto war_clan = account.war_clan.isEmpty() ? current_clan : account.war_clan;
        const auto ranked_key = profile ? RankedKey(*profile) : QString{};
        if (!war_clan.isEmpty()) { request(WarKey(war_clan), ResourceKind::War); request(CwlKey(war_clan), ResourceKind::CwlGroup); }
        if (!ranked_key.isEmpty()) request(ranked_key, ResourceKind::RankedGroup);
        for (const auto& clan : {war_clan, account.cwl_clan}) {
            if (clan.isEmpty()) continue;
            if (clan == account.cwl_clan) request(CwlKey(clan), ResourceKind::CwlGroup);
            if (const auto* existing = value<CwlGroup>(CwlKey(clan))) {
                const auto rounds = existing->rounds;
                for (const auto& round : rounds) for (const auto& tag : round) request(CwlWarKey(tag), ResourceKind::CwlWar);
            }
        }
    }
}

void AccountsModel::received(const QString& key, quint64 generation, const ApiResult& result) {
    if (generation != generation_) return;
    --pending_;
    auto& resource = resources_[key];
    resource.loading = false;
    resource.error = result.error;
    resource.group_not_found_at = result.http_status == 404 && Kind(key) == ResourceKind::CwlGroup
        ? QDateTime::currentDateTimeUtc() : QDateTime{};
    if (result.data) {
        resource.data = result.data;
        resource.fetched = result.fetched;
        resource.from_cache = false;
        QMetaObject::invokeMethod(store_, [store = store_, key, body = result.body, fetched = result.fetched] {
            store->saveCache(key, body, fetched);
        }, Qt::QueuedConnection);
        if (const auto* group = std::get_if<CwlGroup>(&*result.data); group && !group->season.isEmpty()) {
            for (auto& account : accounts_) {
                if (!account.cwl_clan.isEmpty() && key == CwlKey(account.cwl_clan) && account.cwl_season != group->season) {
                    account.cwl_season = group->season;
                    QMetaObject::invokeMethod(store_, [store = store_, tag = account.tag, clan = account.cwl_clan, season = group->season] {
                        store->setCwlSeason(tag, clan, season);
                    }, Qt::QueuedConnection);
                }
            }
        }
    }
    schedule();
    if (pending_ == 0) {
        const auto retained = requested_.values();
        QMetaObject::invokeMethod(store_, [store = store_, retained] { store->pruneCache(retained); }, Qt::QueuedConnection);
        resources_.removeIf([this](QHash<QString, Resource>::iterator it) { return !requested_.contains(it.key()); });
    }
    notifyRows();
}

QVector<RoundWar> AccountsModel::roundWars(const CwlGroup& group, QStringList* dependencies) const {
    QVector<RoundWar> wars;
    for (qsizetype i = 0; i < group.rounds.size(); ++i) {
        for (const auto& tag : group.rounds.at(i)) {
            const auto key = CwlWarKey(tag);
            if (dependencies) dependencies->append(key);
            if (const auto* war = value<War>(key)) wars.append({static_cast<int>(i) + 1, *war, tag});
        }
    }
    return wars;
}

QVariantMap AccountsModel::activityMap(Activity activity, const QStringList& dependencies, bool regularWar) const {
    QStringList errors;
    QDateTime oldest;
    bool loading = false;
    bool stale = false;
    const auto now = QDateTime::currentDateTimeUtc();
    for (const auto& key : dependencies) {
        const auto it = resources_.constFind(key);
        if (it == resources_.cend()) { stale = true; continue; }
        loading = loading || it->loading;
        if (it->group_not_found_at.isValid() && (regularWar || !it->data)) {
            if (it->group_not_found_at.secsTo(now) > (refresh_seconds_ ? refresh_seconds_ : 300)) stale = true;
            if (!oldest.isValid() || it->group_not_found_at < oldest) oldest = it->group_not_found_at;
            continue;
        }
        if (!it->error.isEmpty()) errors.append(it->error);
        if (!it->fetched.isValid() || it->from_cache || !it->error.isEmpty() ||
            it->fetched.secsTo(now) > (refresh_seconds_ ? refresh_seconds_ : 300)) stale = true;
        if (it->fetched.isValid() && (!oldest.isValid() || it->fetched < oldest)) oldest = it->fetched;
    }
    if (activity.end.isValid() && activity.end <= now && activity.state != ActivityState::Ended) stale = true;
    errors.removeDuplicates();
    if (!errors.isEmpty() && !oldest.isValid()) activity.state = ActivityState::Unavailable;
    return {{"status", ActivityText(activity.state)}, {"detail", activity.detail}, {"attention", activity.attention && !stale},
            {"stale", stale}, {"loading", loading}, {"error", errors.join("; ")},
            {"updated", oldest.isValid() ? LocalTime(oldest) : tr("Ещё не получены")},
            {"endUtc", activity.end}, {"end", activity.end.isValid() ? LocalTime(activity.end) : QString{}}, {"source", "Supercell API"}};
}

QVariantMap AccountsModel::row(const AccountSettings& account) const {
    const auto profile_key = PlayerKey(account.tag);
    const auto* profile = value<PlayerProfile>(profile_key);
    const auto clan = ClanTag(profile);
    const auto war_clan = account.war_clan.isEmpty() ? clan : account.war_clan;
    QString participatingWarClan = war_clan;
    Activity ordinary;
    ordinary.state = ActivityState::InsufficientData;
    QStringList war_dependencies;
    if (account.war_clan.isEmpty()) war_dependencies.append(profile_key);
    if (war_clan.isEmpty()) {
        if (profile && !profile->clan) ordinary = Activity{.state = ActivityState::NotParticipating, .detail = tr("Нет текущего клана")};
    } else {
        const auto key = WarKey(war_clan);
        const auto group_key = CwlKey(war_clan);
        war_dependencies.append(key);
        if (const auto* war = value<War>(key)) {
            if (war->opponent.members.contains(account.tag)) participatingWarClan = war->opponent.tag;
            else if (war->clan.members.contains(account.tag)) participatingWarClan = war->clan.tag;
            ordinary = WarActivity(*war, account.tag, QDateTime::currentDateTimeUtc());
            if (war->phase != WarPhase::None && war->phase != WarPhase::Waiting && war->phase != WarPhase::Unknown) {
                war_dependencies.append(group_key);
                const auto groupResource = resources_.constFind(group_key);
                const bool groupMissing = groupResource != resources_.cend() && groupResource->group_not_found_at.isValid();
                if (const auto* group = value<CwlGroup>(group_key); group && !groupMissing) {
                    const auto wars = roundWars(*group, &war_dependencies);
                    bool complete = true;
                    for (const auto& dep : war_dependencies) if (Kind(dep) == ResourceKind::CwlWar && !value<War>(dep)) complete = false;
                    bool matched = false;
                    for (const auto& item : wars) {
                        const auto same = SameWar(*war, item.war);
                        if (same == true) { matched = true; break; }
                        if (!same.has_value()) complete = false;
                    }
                    if (matched) ordinary = Activity{.state = ActivityState::NotParticipating, .detail = tr("Эта война относится к ЛВК")};
                    else if (!complete) ordinary = Activity{.state = ActivityState::InsufficientData, .detail = tr("Тип войны ещё не подтверждён")};
                } else if (!groupMissing) {
                    ordinary = Activity{.state = ActivityState::InsufficientData, .detail = tr("Не удалось отличить КВ от ЛВК")};
                }
            }
        }
    }
    Activity cwl;
    cwl.state = ActivityState::Waiting;
    cwl.detail = tr("Задайте клан ЛВК в настройках аккаунта");
    QStringList cwl_dependencies;
    if (!account.cwl_clan.isEmpty()) {
        const auto key = CwlKey(account.cwl_clan);
        cwl_dependencies.append(key);
        cwl.state = ActivityState::InsufficientData;
        cwl.detail.clear();
        if (const auto* group = value<CwlGroup>(key)) {
            const auto wars = roundWars(*group);
            QStringList neededTags;
            cwl = CwlActivity(*group, wars, account.cwl_clan, account.tag, QDateTime::currentDateTimeUtc(), neededTags);
            for (const auto& tag : neededTags) cwl_dependencies.append(CwlWarKey(tag));
        } else if (resources_.value(key).group_not_found_at.isValid()) {
            cwl = Activity{.state = ActivityState::Unavailable, .detail = tr("Группа ЛВК не найдена; проверьте тег клана")};
        }
    }
    Activity ranked;
    ranked.detail = tr("Нет данных текущего турнира Ranked");
    auto ranked_dependencies = QStringList{profile_key};
    if (profile && profile->league_tier && profile->league_tier->id == 105000036) {
        ranked.detail = tr("Legend I: API не сообщает личный лимит атак на игровые сутки");
    } else if (profile) {
        const auto key = RankedKey(*profile);
        if (!key.isEmpty()) {
            ranked_dependencies.append(key);
            if (const auto* group = value<RankedGroup>(key)) ranked = RankedActivity(*group, *profile, QDateTime::currentDateTimeUtc());
        }
    }
    const auto war_map = activityMap(ordinary, war_dependencies, true);
    const auto cwl_map = activityMap(cwl, cwl_dependencies);
    const auto ranked_map = activityMap(ranked, ranked_dependencies);
    QString action;
    if (!account.transfer_clan.isEmpty()) action = tr("План: перейти в %1%2").arg(account.transfer_clan,
        account.transfer_at.isValid() ? tr(" · %1").arg(LocalTime(account.transfer_at)) : QString{});
    const auto profile_resource = resources_.constFind(profile_key);
    const auto now = QDateTime::currentDateTimeUtc();
    const auto profile_fresh = profile_resource != resources_.cend() && !profile_resource->from_cache &&
        profile_resource->error.isEmpty() && profile_resource->fetched.isValid() &&
        profile_resource->fetched.secsTo(now) <= (refresh_seconds_ ? refresh_seconds_ : 300);
    const bool clan_known = profile_fresh && profile && (!profile->clan || !clan.isEmpty());
    QStringList actions;
    QDateTime deadline;
    auto addDeadline = [&deadline](const QDateTime& time) {
        if (time.isValid() && (!deadline.isValid() || time < deadline)) deadline = time;
    };
    auto describe = [&](const QVariantMap& mode, const QString& label, const QString& targetClan) {
        if (!mode.value("attention").toBool()) return;
        addDeadline(mode.value("endUtc").toDateTime());
        if (clan_known && !targetClan.isEmpty() && targetClan != clan)
            actions.append(tr("Вернуться в клан %1 %2 для атаки").arg(label, targetClan));
        else
            actions.append(tr("Атаковать в %1 %2").arg(label, targetClan).trimmed());
    };
    describe(war_map, tr("КВ"), participatingWarClan);
    describe(cwl_map, tr("ЛВК"), account.cwl_clan);
    describe(ranked_map, tr("Ranked"), {});
    const bool transfer_due = !account.transfer_clan.isEmpty() &&
        (!account.transfer_at.isValid() || account.transfer_at <= now);
    if (transfer_due) addDeadline(account.transfer_at);
    const bool attention = !actions.isEmpty() || transfer_due;
    if (!action.isEmpty()) actions.append(action);
    QString name = profile ? Text(profile->name) : account.tag;
    if (!account.alias.isEmpty()) name = account.alias;
    auto clan_label = clan;
    if (clan.isEmpty()) clan_label = profile && !profile->clan ? tr("Нет клана") : tr("Клан неизвестен");
    else if (profile && profile->clan->name) clan_label = Text(*profile->clan->name) + " · " + clan;
    QString league;
    if (profile && profile->league_tier && profile->league_tier->name) {
        const auto& value = *profile->league_tier->name;
        if (value.is_string()) league = Text(value.get<std::string>());
        else if (value.is_object() && value.contains("en") && value["en"].is_string()) league = Text(value["en"].get<std::string>());
    }
    return {{"tag", account.tag}, {"name", name}, {"clan", clan_label},
            {"league", league}, {"warClan", war_clan}, {"participatingWarClan", participatingWarClan}, {"cwlClan", account.cwl_clan},
            {"war", war_map}, {"cwl", cwl_map}, {"ranked", ranked_map}, {"action", actions.join(" · ")},
            {"deadline", deadline}, {"deadlineLabel", deadline.isValid() ? tr("Ближайший срок: %1").arg(LocalTime(deadline)) : QString{}}, {"attention", attention}};
}
