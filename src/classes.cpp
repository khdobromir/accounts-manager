#include "classes.hpp"
#include "config.hpp"
#include "json.hpp"

#include <cpr/cpr.h>

#include <utility>
#include <print>

Player::Player(std::string tag)
    : tag_(std::move(tag)) {
        try {
            std::string API_KEY = GetEnv("API_KEY");
            auto response = cpr::Get(
                cpr::Url{std::format("https://api.clashofclans.com/v1/players/%23{}", tag_.substr(1))},
                cpr::Bearer{API_KEY},
                cpr::Timeout{10000}
            );
            if (response.error.code != cpr::ErrorCode::OK) {
                std::println("error in Player's constructor: network error: {}", response.error.message);
            }
            if (response.status_code < 200 || response.status_code >= 300) {
                std::println("error in Player's constructor: HTTP error, code: {}", response.status_code);
            }
            name_ = GetPlayerInfo(response.text, "name");
            Clan clan = GetPlayerClanInfo(response.text);
            //clan_name_ = GetPlayerInfo(GetPlayerInfo(response.text, "clan"), "name");
            //clan_tag_ = GetPlayerInfo(GetPlayerInfo(response.text, "clan"), "tag");

        } catch (std::exception& e) {
            std::println("error in Player's constructor: {}", e.what());
        }
    }

const std::string& Player::tag() const {
    return tag_;
}

const std::string& Player::name() const {
    return name_;
}

const std::string& Player::clan_tag() const {
    return clan_tag_;
}

const std::string& Player::clan_name() const {
    return clan_name_;
}