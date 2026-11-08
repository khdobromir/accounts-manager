#include "json.hpp"

#include <nlohmann/json.hpp>

#include <string>
#include <print>

using json = nlohmann::json;

const std::string GetPlayerInfo(std::string source, std::string key) {
    try {
        json data = json::parse(source);
        return data.at(key).get<std::string>();
    } catch (const json::exception& e) {
        std::println("GetName: JSON parsing error: {}\n", e.what());
        return "";
    }
}

Clan GetPlayerClanInfo(std::string source) {
        try {
        json clan = json::parse(source).at("clan");
        return Clan{
            .clanLevel = clan.at("clanLevel").get<size_t>(),
            .name = clan.at("name").get<std::string>(),
            .tag = clan.at("tag").get<std::string>(),
        };
    } catch (const json::exception& e) {
        std::println("GetName: JSON parsing error: {}\n", e.what());
        return Clan{};
    }
}