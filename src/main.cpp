#include "classes.hpp"

#include <cstdio>
#include <exception>
#include <format>
#include <fstream>
#include <print>
#include <string_view>

namespace {

template <typename T>
void PrintField(std::ostream& output, std::string_view field, const T& value) {
    output << field << " - " << value << '\n';
}

void PrintField(std::ostream& output, std::string_view field, const nlohmann::json& value) {
    output << field << " - " << value.dump() << '\n';
}

void PrintField(std::ostream& output, std::string_view field, const Clan& value);
void PrintField(std::ostream& output, std::string_view field, const League& value);
void PrintField(std::ostream& output, std::string_view field, const LeagueTier& value);
void PrintField(std::ostream& output, std::string_view field, const BuilderBaseLeague& value);
void PrintField(std::ostream& output, std::string_view field, const PlayerAchievementProgress& value);
void PrintField(std::ostream& output, std::string_view field, const PlayerItemLevel& value);
void PrintField(std::ostream& output, std::string_view field, const LegendLeagueTournamentSeasonResult& value);
void PrintField(std::ostream& output, std::string_view field, const PlayerLegendStatistics& value);
void PrintField(std::ostream& output, std::string_view field, const Label& value);
void PrintField(std::ostream& output, std::string_view field, const PlayerHouseElement& value);
void PrintField(std::ostream& output, std::string_view field, const PlayerHouse& value);

template <typename T>
void PrintField(std::ostream& output, std::string_view field, const std::vector<T>& values) {
    if (values.empty()) {
        output << field << " - []\n";
    }
    for (std::size_t i = 0; i < values.size(); ++i) {
        PrintField(output, std::format("{}[{}]", field, i), values[i]);
    }
}

template <typename T>
void PrintField(std::ostream& output, std::string_view field, const std::optional<T>& value) {
    if (value) {
        PrintField(output, field, *value);
    } else {
        output << field << " - null\n";
    }
}

void PrintField(std::ostream& output, std::string_view field, const Clan& value) {
    PrintField(output, std::format("{}.clan_level", field), value.clan_level);
    PrintField(output, std::format("{}.name", field), value.name);
    PrintField(output, std::format("{}.tag", field), value.tag);
    PrintField(output, std::format("{}.badge_urls", field), value.badge_urls);
}

void PrintField(std::ostream& output, std::string_view field, const League& value) {
    PrintField(output, std::format("{}.name", field), value.name);
    PrintField(output, std::format("{}.id", field), value.id);
    PrintField(output, std::format("{}.icon_urls", field), value.icon_urls);
}

void PrintField(std::ostream& output, std::string_view field, const LeagueTier& value) {
    PrintField(output, std::format("{}.name", field), value.name);
    PrintField(output, std::format("{}.id", field), value.id);
    PrintField(output, std::format("{}.icon_urls", field), value.icon_urls);
}

void PrintField(std::ostream& output, std::string_view field, const BuilderBaseLeague& value) {
    PrintField(output, std::format("{}.name", field), value.name);
    PrintField(output, std::format("{}.id", field), value.id);
}

void PrintField(std::ostream& output, std::string_view field, const PlayerAchievementProgress& value) {
    PrintField(output, std::format("{}.stars", field), value.stars);
    PrintField(output, std::format("{}.value", field), value.value);
    PrintField(output, std::format("{}.name", field), value.name);
    PrintField(output, std::format("{}.target", field), value.target);
    PrintField(output, std::format("{}.info", field), value.info);
    PrintField(output, std::format("{}.completion_info", field), value.completion_info);
    PrintField(output, std::format("{}.village", field), value.village);
}

void PrintField(std::ostream& output, std::string_view field, const PlayerItemLevel& value) {
    PrintField(output, std::format("{}.level", field), value.level);
    PrintField(output, std::format("{}.name", field), value.name);
    PrintField(output, std::format("{}.max_level", field), value.max_level);
    PrintField(output, std::format("{}.village", field), value.village);
    PrintField(output, std::format("{}.super_troop_is_active", field), value.super_troop_is_active);
    PrintField(output, std::format("{}.equipment", field), value.equipment);
}

void PrintField(std::ostream& output, std::string_view field, const LegendLeagueTournamentSeasonResult& value) {
    PrintField(output, std::format("{}.trophies", field), value.trophies);
    PrintField(output, std::format("{}.id", field), value.id);
    PrintField(output, std::format("{}.rank", field), value.rank);
}

void PrintField(std::ostream& output, std::string_view field, const PlayerLegendStatistics& value) {
    PrintField(output, std::format("{}.previous_builder_base_season", field), value.previous_builder_base_season);
    PrintField(output, std::format("{}.current_season", field), value.current_season);
    PrintField(output, std::format("{}.previous_season", field), value.previous_season);
    PrintField(output, std::format("{}.best_season", field), value.best_season);
    PrintField(output, std::format("{}.best_builder_base_season", field), value.best_builder_base_season);
    PrintField(output, std::format("{}.legend_trophies", field), value.legend_trophies);
}

void PrintField(std::ostream& output, std::string_view field, const Label& value) {
    PrintField(output, std::format("{}.name", field), value.name);
    PrintField(output, std::format("{}.id", field), value.id);
    PrintField(output, std::format("{}.icon_urls", field), value.icon_urls);
}

void PrintField(std::ostream& output, std::string_view field, const PlayerHouseElement& value) {
    PrintField(output, std::format("{}.id", field), value.id);
    PrintField(output, std::format("{}.type", field), value.type);
}

void PrintField(std::ostream& output, std::string_view field, const PlayerHouse& value) {
    PrintField(output, std::format("{}.elements", field), value.elements);
}

} // namespace

int main() {
    try {
        Player player1("#Q2UV9VJ8J");
        std::ofstream output;
        output.exceptions(std::ios::failbit | std::ios::badbit);
        output.open("player_profile.txt", std::ios::out | std::ios::trunc);
        output << std::boolalpha;
        PrintField(output, "tag", player1.tag());
        PrintField(output, "name", player1.name());
        PrintField(output, "clan", player1.clan());
        PrintField(output, "league", player1.league());
        PrintField(output, "league_tier", player1.league_tier());
        PrintField(output, "builder_base_league", player1.builder_base_league());
        PrintField(output, "role", player1.role());
        PrintField(output, "war_preference", player1.war_preference());
        PrintField(output, "attack_wins", player1.attack_wins());
        PrintField(output, "defense_wins", player1.defense_wins());
        PrintField(output, "town_hall_level", player1.town_hall_level());
        PrintField(output, "town_hall_weapon_level", player1.town_hall_weapon_level());
        PrintField(output, "legend_statistics", player1.legend_statistics());
        PrintField(output, "troops", player1.troops());
        PrintField(output, "heroes", player1.heroes());
        PrintField(output, "hero_equipment", player1.hero_equipment());
        PrintField(output, "spells", player1.spells());
        PrintField(output, "labels", player1.labels());
        PrintField(output, "exp_level", player1.exp_level());
        PrintField(output, "trophies", player1.trophies());
        PrintField(output, "best_trophies", player1.best_trophies());
        PrintField(output, "donations", player1.donations());
        PrintField(output, "donations_received", player1.donations_received());
        PrintField(output, "builder_hall_level", player1.builder_hall_level());
        PrintField(output, "builder_base_trophies", player1.builder_base_trophies());
        PrintField(output, "best_builder_base_trophies", player1.best_builder_base_trophies());
        PrintField(output, "war_stars", player1.war_stars());
        PrintField(output, "achievements", player1.achievements());
        PrintField(output, "clan_capital_contributions", player1.clan_capital_contributions());
        PrintField(output, "player_house", player1.player_house());
        PrintField(output, "current_league_group_tag", player1.current_league_group_tag());
        PrintField(output, "current_league_season_id", player1.current_league_season_id());
        PrintField(output, "previous_league_group_tag", player1.previous_league_group_tag());
        PrintField(output, "previous_league_season_id", player1.previous_league_season_id());
        output.close();
        std::println("Profile saved to player_profile.txt");
        return 0;
    } catch (const std::exception& e) {
        std::println(stderr, "Error: {}", e.what());
        return 1;
    }
}
