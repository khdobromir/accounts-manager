#pragma once

#include "player_types.hpp"

#include <QByteArray>
#include <QDateTime>
#include <QHash>
#include <QMetaType>
#include <QSet>
#include <QString>
#include <QVector>

#include <optional>
#include <variant>

enum class ResourceKind { Player, War, CwlGroup, CwlWar, RankedGroup };
enum class WarPhase { None, Waiting, Preparation, Active, Ended, Unknown };
enum class ActivityState { AttacksRemaining, Done, NotParticipating, Waiting, Unavailable, InsufficientData, Ended };

struct WarSide {
    QString tag = {};
    QHash<QString, std::optional<std::int64_t>> members;
    bool roster_known = false;
};

struct War {
    WarPhase phase = WarPhase::Unknown;
    std::optional<std::int64_t> limit = std::nullopt;
    QDateTime start;
    QDateTime end;
    WarSide clan;
    WarSide opponent;
};

struct CwlGroup {
    QString season = {};
    WarPhase phase = WarPhase::Unknown;
    QHash<QString, QSet<QString>> rosters;
    QVector<QStringList> rounds;
};

struct RankedGroup {
    QString period = {};
    std::optional<std::int64_t> limit = std::nullopt;
    QHash<QString, std::optional<std::int64_t>> members;
};

using ApiData = std::variant<PlayerProfile, War, CwlGroup, RankedGroup>;

struct ApiResult {
    std::optional<ApiData> data = std::nullopt;
    QByteArray body = {};
    QDateTime fetched = {};
    QString error = {};
    int http_status = 0;
};
Q_DECLARE_METATYPE(ApiResult)

struct Activity {
    ActivityState state = ActivityState::InsufficientData;
    QString detail = {};
    std::optional<std::int64_t> used = std::nullopt;
    std::optional<std::int64_t> limit = std::nullopt;
    QDateTime end = {};
    bool attention = false;
};

struct RoundWar {
    int round = 0;
    War war;
    QString tag = {};
};

QString NormalizeTag(QString tag);
QString EncodedTag(const QString& tag);
QString PlayerKey(const QString& tag);
QString WarKey(const QString& clan);
QString CwlKey(const QString& clan);
ApiData ParseResource(ResourceKind kind, const QByteArray& body);
Activity WarActivity(const War& war, const QString& player, const QDateTime& now);
Activity CwlActivity(const CwlGroup& group, const QVector<RoundWar>& wars,
                     const QString& clan, const QString& player, const QDateTime& now, QStringList& dependencies);
Activity RankedActivity(const RankedGroup& group, const PlayerProfile& profile, const QDateTime& now);
QString ActivityText(ActivityState state);
std::optional<bool> SameWar(const War& a, const War& b);
bool ResourceMatches(const QString& key, const ApiData& data);

struct AccountSettings {
    QString tag;
    QString alias;
    QString war_clan;
    QString cwl_clan;
    QString cwl_season;
    QString transfer_clan;
    QDateTime transfer_at;
};
Q_DECLARE_METATYPE(AccountSettings)

struct CacheEntry {
    QByteArray body;
    QDateTime fetched;
};

struct StoredState {
    QList<AccountSettings> accounts;
    QHash<QString, CacheEntry> cache;
    int refresh_seconds = 300;
};
Q_DECLARE_METATYPE(StoredState)
