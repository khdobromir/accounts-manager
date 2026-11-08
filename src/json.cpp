#include "json.hpp"

#include <cstddef>
#include <format>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace {

using json = nlohmann::json;

[[noreturn]] void InvalidField(const std::string& path, std::string_view expected) {
    throw std::runtime_error(std::format("Invalid player profile at {}: expected {}", path, expected));
}

void RequireObject(const json& value, const std::string& path) {
    if (!value.is_object()) {
        InvalidField(path, "an object");
    }
}

Clan ReadObject(const json& value, const std::string& path, std::type_identity<Clan>);
League ReadObject(const json& value, const std::string& path, std::type_identity<League>);
LeagueTier ReadObject(const json& value, const std::string& path, std::type_identity<LeagueTier>);
BuilderBaseLeague ReadObject(const json& value, const std::string& path, std::type_identity<BuilderBaseLeague>);
PlayerAchievementProgress ReadObject(const json& value, const std::string& path, std::type_identity<PlayerAchievementProgress>);
PlayerItemLevel ReadObject(const json& value, const std::string& path, std::type_identity<PlayerItemLevel>);
LegendLeagueTournamentSeasonResult ReadObject(const json& value, const std::string& path, std::type_identity<LegendLeagueTournamentSeasonResult>);
PlayerLegendStatistics ReadObject(const json& value, const std::string& path, std::type_identity<PlayerLegendStatistics>);
Label ReadObject(const json& value, const std::string& path, std::type_identity<Label>);
PlayerHouseElement ReadObject(const json& value, const std::string& path, std::type_identity<PlayerHouseElement>);
PlayerHouse ReadObject(const json& value, const std::string& path, std::type_identity<PlayerHouse>);

template <typename T>
T ReadValue(const json& value, const std::string& path) {
    if constexpr (std::is_same_v<T, json>) {
        return value;
    } else if constexpr (std::is_same_v<T, std::string>) {
        if (!value.is_string()) {
            InvalidField(path, "a string");
        }
        return value.get<std::string>();
    } else if constexpr (std::is_same_v<T, bool>) {
        if (!value.is_boolean()) {
            InvalidField(path, "a boolean");
        }
        return value.get<bool>();
    } else if constexpr (std::is_same_v<T, std::int64_t>) {
        if (value.is_number_unsigned()) {
            const auto number = value.get<std::uint64_t>();
            if (number > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
                InvalidField(path, "an integer within the int64_t range");
            }
            return static_cast<std::int64_t>(number);
        }
        if (!value.is_number_integer()) {
            InvalidField(path, "an integer within the int64_t range");
        }
        return value.get<std::int64_t>();
    } else {
        RequireObject(value, path);
        return ReadObject(value, path, std::type_identity<T>{});
    }
}

template <typename T>
std::optional<T> ReadOptional(const json& object, std::string_view key, const std::string& path) {
    const auto it = object.find(key);
    if (it == object.end() || it->is_null()) {
        return std::nullopt;
    }
    return std::optional<T>{std::in_place, ReadValue<T>(*it, std::format("{}.{}", path, key))};
}

template <typename T>
std::optional<std::vector<T>> ReadOptionalArray(const json& object, std::string_view key, const std::string& path) {
    const auto it = object.find(key);
    if (it == object.end() || it->is_null()) {
        return std::nullopt;
    }
    const auto field_path = std::format("{}.{}", path, key);
    if (!it->is_array()) {
        InvalidField(field_path, "an array");
    }
    std::vector<T> result;
    result.reserve(it->size());
    for (std::size_t i = 0; i < it->size(); ++i) {
        result.push_back(ReadValue<T>((*it)[i], std::format("{}[{}]", field_path, i)));
    }
    return result;
}

std::optional<json> ReadOptionalJsonObject(const json& object, std::string_view key, const std::string& path) {
    const auto it = object.find(key);
    if (it == object.end() || it->is_null()) {
        return std::nullopt;
    }
    RequireObject(*it, std::format("{}.{}", path, key));
    return std::optional<json>{std::in_place, *it};
}

std::string ReadRequiredString(const json& object, std::string_view key) {
    const auto it = object.find(key);
    const auto path = std::format("$.{}", key);
    if (it == object.end()) {
        InvalidField(path, "a required string");
    }
    return ReadValue<std::string>(*it, path);
}

Clan ReadObject(const json& value, const std::string& path, std::type_identity<Clan>) {
    return Clan{
        .clan_level = ReadOptional<std::int64_t>(value, "clanLevel", path),
        .name = ReadOptional<std::string>(value, "name", path),
        .tag = ReadOptional<std::string>(value, "tag", path),
        .badge_urls = ReadOptionalJsonObject(value, "badgeUrls", path),
    };
}

League ReadObject(const json& value, const std::string& path, std::type_identity<League>) {
    return League{
        .name = ReadOptional<json>(value, "name", path),
        .id = ReadOptional<std::int64_t>(value, "id", path),
        .icon_urls = ReadOptionalJsonObject(value, "iconUrls", path),
    };
}

LeagueTier ReadObject(const json& value, const std::string& path, std::type_identity<LeagueTier>) {
    return LeagueTier{
        .name = ReadOptional<json>(value, "name", path),
        .id = ReadOptional<std::int64_t>(value, "id", path),
        .icon_urls = ReadOptionalJsonObject(value, "iconUrls", path),
    };
}

BuilderBaseLeague ReadObject(const json& value, const std::string& path, std::type_identity<BuilderBaseLeague>) {
    return BuilderBaseLeague{
        .name = ReadOptional<json>(value, "name", path),
        .id = ReadOptional<std::int64_t>(value, "id", path),
    };
}

PlayerAchievementProgress ReadObject(const json& value, const std::string& path, std::type_identity<PlayerAchievementProgress>) {
    return PlayerAchievementProgress{
        .stars = ReadOptional<std::int64_t>(value, "stars", path),
        .value = ReadOptional<std::int64_t>(value, "value", path),
        .name = ReadOptional<json>(value, "name", path),
        .target = ReadOptional<std::int64_t>(value, "target", path),
        .info = ReadOptional<json>(value, "info", path),
        .completion_info = ReadOptional<json>(value, "completionInfo", path),
        .village = ReadOptional<std::string>(value, "village", path),
    };
}

PlayerItemLevel ReadObject(const json& value, const std::string& path, std::type_identity<PlayerItemLevel>) {
    return PlayerItemLevel{
        .level = ReadOptional<std::int64_t>(value, "level", path),
        .name = ReadOptional<json>(value, "name", path),
        .max_level = ReadOptional<std::int64_t>(value, "maxLevel", path),
        .village = ReadOptional<std::string>(value, "village", path),
        .super_troop_is_active = ReadOptional<bool>(value, "superTroopIsActive", path),
        .equipment = ReadOptionalArray<PlayerItemLevel>(value, "equipment", path),
    };
}

LegendLeagueTournamentSeasonResult ReadObject(const json& value, const std::string& path, std::type_identity<LegendLeagueTournamentSeasonResult>) {
    return LegendLeagueTournamentSeasonResult{
        .trophies = ReadOptional<std::int64_t>(value, "trophies", path),
        .id = ReadOptional<std::string>(value, "id", path),
        .rank = ReadOptional<std::int64_t>(value, "rank", path),
    };
}

PlayerLegendStatistics ReadObject(const json& value, const std::string& path, std::type_identity<PlayerLegendStatistics>) {
    return PlayerLegendStatistics{
        .previous_builder_base_season = ReadOptional<LegendLeagueTournamentSeasonResult>(value, "previousBuilderBaseSeason", path),
        .current_season = ReadOptional<LegendLeagueTournamentSeasonResult>(value, "currentSeason", path),
        .previous_season = ReadOptional<LegendLeagueTournamentSeasonResult>(value, "previousSeason", path),
        .best_season = ReadOptional<LegendLeagueTournamentSeasonResult>(value, "bestSeason", path),
        .best_builder_base_season = ReadOptional<LegendLeagueTournamentSeasonResult>(value, "bestBuilderBaseSeason", path),
        .legend_trophies = ReadOptional<std::int64_t>(value, "legendTrophies", path),
    };
}

Label ReadObject(const json& value, const std::string& path, std::type_identity<Label>) {
    return Label{
        .name = ReadOptional<json>(value, "name", path),
        .id = ReadOptional<std::int64_t>(value, "id", path),
        .icon_urls = ReadOptionalJsonObject(value, "iconUrls", path),
    };
}

PlayerHouseElement ReadObject(const json& value, const std::string& path, std::type_identity<PlayerHouseElement>) {
    return PlayerHouseElement{
        .id = ReadOptional<std::int64_t>(value, "id", path),
        .type = ReadOptional<std::string>(value, "type", path),
    };
}

PlayerHouse ReadObject(const json& value, const std::string& path, std::type_identity<PlayerHouse>) {
    return PlayerHouse{
        .elements = ReadOptionalArray<PlayerHouseElement>(value, "elements", path),
    };
}

} // namespace

PlayerProfile ParsePlayerProfile(std::string_view source) {
    json data;
    try {
        data = json::parse(source.begin(), source.end());
    } catch (const json::exception& e) {
        throw std::runtime_error(std::format("Invalid player profile at $: {}", e.what()));
    }
    RequireObject(data, "$");
    auto tag = ReadRequiredString(data, "tag");
    if (tag.empty()) {
        InvalidField("$.tag", "a non-empty string");
    }
    auto name = ReadRequiredString(data, "name");

    return PlayerProfile{
        .clan = ReadOptional<Clan>(data, "clan", "$"),
        .league = ReadOptional<League>(data, "league", "$"),
        .league_tier = ReadOptional<LeagueTier>(data, "leagueTier", "$"),
        .builder_base_league = ReadOptional<BuilderBaseLeague>(data, "builderBaseLeague", "$"),
        .role = ReadOptional<std::string>(data, "role", "$"),
        .war_preference = ReadOptional<std::string>(data, "warPreference", "$"),
        .attack_wins = ReadOptional<std::int64_t>(data, "attackWins", "$"),
        .defense_wins = ReadOptional<std::int64_t>(data, "defenseWins", "$"),
        .town_hall_level = ReadOptional<std::int64_t>(data, "townHallLevel", "$"),
        .town_hall_weapon_level = ReadOptional<std::int64_t>(data, "townHallWeaponLevel", "$"),
        .legend_statistics = ReadOptional<PlayerLegendStatistics>(data, "legendStatistics", "$"),
        .troops = ReadOptionalArray<PlayerItemLevel>(data, "troops", "$"),
        .heroes = ReadOptionalArray<PlayerItemLevel>(data, "heroes", "$"),
        .hero_equipment = ReadOptionalArray<PlayerItemLevel>(data, "heroEquipment", "$"),
        .spells = ReadOptionalArray<PlayerItemLevel>(data, "spells", "$"),
        .labels = ReadOptionalArray<Label>(data, "labels", "$"),
        .tag = std::move(tag),
        .name = std::move(name),
        .exp_level = ReadOptional<std::int64_t>(data, "expLevel", "$"),
        .trophies = ReadOptional<std::int64_t>(data, "trophies", "$"),
        .best_trophies = ReadOptional<std::int64_t>(data, "bestTrophies", "$"),
        .donations = ReadOptional<std::int64_t>(data, "donations", "$"),
        .donations_received = ReadOptional<std::int64_t>(data, "donationsReceived", "$"),
        .builder_hall_level = ReadOptional<std::int64_t>(data, "builderHallLevel", "$"),
        .builder_base_trophies = ReadOptional<std::int64_t>(data, "builderBaseTrophies", "$"),
        .best_builder_base_trophies = ReadOptional<std::int64_t>(data, "bestBuilderBaseTrophies", "$"),
        .war_stars = ReadOptional<std::int64_t>(data, "warStars", "$"),
        .achievements = ReadOptionalArray<PlayerAchievementProgress>(data, "achievements", "$"),
        .clan_capital_contributions = ReadOptional<std::int64_t>(data, "clanCapitalContributions", "$"),
        .player_house = ReadOptional<PlayerHouse>(data, "playerHouse", "$"),
        .current_league_group_tag = ReadOptional<std::string>(data, "currentLeagueGroupTag", "$"),
        .current_league_season_id = ReadOptional<json>(data, "currentLeagueSeasonId", "$"),
        .previous_league_group_tag = ReadOptional<std::string>(data, "previousLeagueGroupTag", "$"),
        .previous_league_season_id = ReadOptional<json>(data, "previousLeagueSeasonId", "$"),
    };
}
