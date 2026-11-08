#pragma once

#include "player_types.hpp"

class Player {
private:
    PlayerProfile profile_;

public:
    Player(std::string tag);

    const std::optional<Clan>& clan() const;
    const std::optional<League>& league() const;
    const std::optional<LeagueTier>& league_tier() const;
    const std::optional<BuilderBaseLeague>& builder_base_league() const;
    const std::optional<std::string>& role() const;
    const std::optional<std::string>& war_preference() const;
    std::optional<std::int64_t> attack_wins() const;
    std::optional<std::int64_t> defense_wins() const;
    std::optional<std::int64_t> town_hall_level() const;
    std::optional<std::int64_t> town_hall_weapon_level() const;
    const std::optional<PlayerLegendStatistics>& legend_statistics() const;
    const std::optional<std::vector<PlayerItemLevel>>& troops() const;
    const std::optional<std::vector<PlayerItemLevel>>& heroes() const;
    const std::optional<std::vector<PlayerItemLevel>>& hero_equipment() const;
    const std::optional<std::vector<PlayerItemLevel>>& spells() const;
    const std::optional<std::vector<Label>>& labels() const;
    const std::string& tag() const;
    const std::string& name() const;
    std::optional<std::int64_t> exp_level() const;
    std::optional<std::int64_t> trophies() const;
    std::optional<std::int64_t> best_trophies() const;
    std::optional<std::int64_t> donations() const;
    std::optional<std::int64_t> donations_received() const;
    std::optional<std::int64_t> builder_hall_level() const;
    std::optional<std::int64_t> builder_base_trophies() const;
    std::optional<std::int64_t> best_builder_base_trophies() const;
    std::optional<std::int64_t> war_stars() const;
    const std::optional<std::vector<PlayerAchievementProgress>>& achievements() const;
    std::optional<std::int64_t> clan_capital_contributions() const;
    const std::optional<PlayerHouse>& player_house() const;
    const std::optional<std::string>& current_league_group_tag() const;
    const std::optional<nlohmann::json>& current_league_season_id() const;
    const std::optional<std::string>& previous_league_group_tag() const;
    const std::optional<nlohmann::json>& previous_league_season_id() const;

    const std::string& clan_tag() const;
    const std::string& clan_name() const;
};
