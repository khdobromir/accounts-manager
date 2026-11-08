#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct Clan {
    std::optional<std::int64_t> clan_level;
    std::optional<std::string> name;
    std::optional<std::string> tag;
    std::optional<nlohmann::json> badge_urls;
};

struct League {
    std::optional<nlohmann::json> name;
    std::optional<std::int64_t> id;
    std::optional<nlohmann::json> icon_urls;
};

struct LeagueTier {
    std::optional<nlohmann::json> name;
    std::optional<std::int64_t> id;
    std::optional<nlohmann::json> icon_urls;
};

struct BuilderBaseLeague {
    std::optional<nlohmann::json> name;
    std::optional<std::int64_t> id;
};

struct PlayerAchievementProgress {
    std::optional<std::int64_t> stars;
    std::optional<std::int64_t> value;
    std::optional<nlohmann::json> name;
    std::optional<std::int64_t> target;
    std::optional<nlohmann::json> info;
    std::optional<nlohmann::json> completion_info;
    std::optional<std::string> village;
};

struct PlayerItemLevel {
    std::optional<std::int64_t> level;
    std::optional<nlohmann::json> name;
    std::optional<std::int64_t> max_level;
    std::optional<std::string> village;
    std::optional<bool> super_troop_is_active;
    std::optional<std::vector<PlayerItemLevel>> equipment;
};

struct LegendLeagueTournamentSeasonResult {
    std::optional<std::int64_t> trophies;
    std::optional<std::string> id;
    std::optional<std::int64_t> rank;
};

struct PlayerLegendStatistics {
    std::optional<LegendLeagueTournamentSeasonResult> previous_builder_base_season;
    std::optional<LegendLeagueTournamentSeasonResult> current_season;
    std::optional<LegendLeagueTournamentSeasonResult> previous_season;
    std::optional<LegendLeagueTournamentSeasonResult> best_season;
    std::optional<LegendLeagueTournamentSeasonResult> best_builder_base_season;
    std::optional<std::int64_t> legend_trophies;
};

struct Label {
    std::optional<nlohmann::json> name;
    std::optional<std::int64_t> id;
    std::optional<nlohmann::json> icon_urls;
};

struct PlayerHouseElement {
    std::optional<std::int64_t> id;
    std::optional<std::string> type;
};

struct PlayerHouse {
    std::optional<std::vector<PlayerHouseElement>> elements;
};

struct PlayerProfile {
    std::optional<Clan> clan;
    std::optional<League> league;
    std::optional<LeagueTier> league_tier;
    std::optional<BuilderBaseLeague> builder_base_league;
    std::optional<std::string> role;
    std::optional<std::string> war_preference;
    std::optional<std::int64_t> attack_wins;
    std::optional<std::int64_t> defense_wins;
    std::optional<std::int64_t> town_hall_level;
    std::optional<std::int64_t> town_hall_weapon_level;
    std::optional<PlayerLegendStatistics> legend_statistics;
    std::optional<std::vector<PlayerItemLevel>> troops;
    std::optional<std::vector<PlayerItemLevel>> heroes;
    std::optional<std::vector<PlayerItemLevel>> hero_equipment;
    std::optional<std::vector<PlayerItemLevel>> spells;
    std::optional<std::vector<Label>> labels;
    std::string tag;
    std::string name;
    std::optional<std::int64_t> exp_level;
    std::optional<std::int64_t> trophies;
    std::optional<std::int64_t> best_trophies;
    std::optional<std::int64_t> donations;
    std::optional<std::int64_t> donations_received;
    std::optional<std::int64_t> builder_hall_level;
    std::optional<std::int64_t> builder_base_trophies;
    std::optional<std::int64_t> best_builder_base_trophies;
    std::optional<std::int64_t> war_stars;
    std::optional<std::vector<PlayerAchievementProgress>> achievements;
    std::optional<std::int64_t> clan_capital_contributions;
    std::optional<PlayerHouse> player_house;
    std::optional<std::string> current_league_group_tag;
    std::optional<nlohmann::json> current_league_season_id;
    std::optional<std::string> previous_league_group_tag;
    std::optional<nlohmann::json> previous_league_season_id;
};
