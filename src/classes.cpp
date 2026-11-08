#include "classes.hpp"
#include "config.hpp"
#include "json.hpp"

#include <cpr/cpr.h>
#include <cpr/util.h>

#include <format>
#include <stdexcept>

namespace {

std::string HttpErrorMessage(const cpr::Response& response) {
    auto message = std::format("Player request failed: HTTP {}", response.status_code);
    try {
        const auto error = nlohmann::json::parse(response.text);
        if (error.is_object()) {
            for (const auto* key : {"reason", "message"}) {
                const auto it = error.find(key);
                if (it != error.end() && it->is_string()) {
                    message += std::format("; {}: {}", key, it->get_ref<const std::string&>());
                }
            }
        }
    } catch (const nlohmann::json::exception&) {
    }
    return message;
}

PlayerProfile LoadPlayerProfile(const std::string& tag) {
    if (tag.size() < 2 || tag.front() != '#') {
        throw std::invalid_argument("Player tag must start with '#' and contain a non-empty identifier");
    }

    const auto encoded_tag = cpr::util::urlEncode(tag);
    const auto api_key = GetEnv("API_KEY");
    const auto response = cpr::Get(
        cpr::Url{std::format("https://api.clashofclans.com/v1/players/{}", encoded_tag.c_str())},
        cpr::Bearer{api_key},
        cpr::Timeout{10000}
    );
    if (response.error.code != cpr::ErrorCode::OK) {
        throw std::runtime_error(std::format("Player request failed: network error: {}", response.error.message));
    }
    if (response.status_code != 200) {
        throw std::runtime_error(HttpErrorMessage(response));
    }
    return ParsePlayerProfile(response.text);
}

} // namespace

Player::Player(std::string tag)
    : profile_(LoadPlayerProfile(tag)) {
}

const std::optional<Clan>& Player::clan() const {
    return profile_.clan;
}

const std::optional<League>& Player::league() const {
    return profile_.league;
}

const std::optional<LeagueTier>& Player::league_tier() const {
    return profile_.league_tier;
}

const std::optional<BuilderBaseLeague>& Player::builder_base_league() const {
    return profile_.builder_base_league;
}

const std::optional<std::string>& Player::role() const {
    return profile_.role;
}

const std::optional<std::string>& Player::war_preference() const {
    return profile_.war_preference;
}

std::optional<std::int64_t> Player::attack_wins() const {
    return profile_.attack_wins;
}

std::optional<std::int64_t> Player::defense_wins() const {
    return profile_.defense_wins;
}

std::optional<std::int64_t> Player::town_hall_level() const {
    return profile_.town_hall_level;
}

std::optional<std::int64_t> Player::town_hall_weapon_level() const {
    return profile_.town_hall_weapon_level;
}

const std::optional<PlayerLegendStatistics>& Player::legend_statistics() const {
    return profile_.legend_statistics;
}

const std::optional<std::vector<PlayerItemLevel>>& Player::troops() const {
    return profile_.troops;
}

const std::optional<std::vector<PlayerItemLevel>>& Player::heroes() const {
    return profile_.heroes;
}

const std::optional<std::vector<PlayerItemLevel>>& Player::hero_equipment() const {
    return profile_.hero_equipment;
}

const std::optional<std::vector<PlayerItemLevel>>& Player::spells() const {
    return profile_.spells;
}

const std::optional<std::vector<Label>>& Player::labels() const {
    return profile_.labels;
}

const std::string& Player::tag() const {
    return profile_.tag;
}

const std::string& Player::name() const {
    return profile_.name;
}

std::optional<std::int64_t> Player::exp_level() const {
    return profile_.exp_level;
}

std::optional<std::int64_t> Player::trophies() const {
    return profile_.trophies;
}

std::optional<std::int64_t> Player::best_trophies() const {
    return profile_.best_trophies;
}

std::optional<std::int64_t> Player::donations() const {
    return profile_.donations;
}

std::optional<std::int64_t> Player::donations_received() const {
    return profile_.donations_received;
}

std::optional<std::int64_t> Player::builder_hall_level() const {
    return profile_.builder_hall_level;
}

std::optional<std::int64_t> Player::builder_base_trophies() const {
    return profile_.builder_base_trophies;
}

std::optional<std::int64_t> Player::best_builder_base_trophies() const {
    return profile_.best_builder_base_trophies;
}

std::optional<std::int64_t> Player::war_stars() const {
    return profile_.war_stars;
}

const std::optional<std::vector<PlayerAchievementProgress>>& Player::achievements() const {
    return profile_.achievements;
}

std::optional<std::int64_t> Player::clan_capital_contributions() const {
    return profile_.clan_capital_contributions;
}

const std::optional<PlayerHouse>& Player::player_house() const {
    return profile_.player_house;
}

const std::optional<std::string>& Player::current_league_group_tag() const {
    return profile_.current_league_group_tag;
}

const std::optional<nlohmann::json>& Player::current_league_season_id() const {
    return profile_.current_league_season_id;
}

const std::optional<std::string>& Player::previous_league_group_tag() const {
    return profile_.previous_league_group_tag;
}

const std::optional<nlohmann::json>& Player::previous_league_season_id() const {
    return profile_.previous_league_season_id;
}

const std::string& Player::clan_tag() const {
    static const std::string empty;
    if (profile_.clan && profile_.clan->tag) {
        return *profile_.clan->tag;
    }
    return empty;
}

const std::string& Player::clan_name() const {
    static const std::string empty;
    if (profile_.clan && profile_.clan->name) {
        return *profile_.clan->name;
    }
    return empty;
}
