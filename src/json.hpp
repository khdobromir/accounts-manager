#pragma once

#include <string>

struct Clan {
    size_t clanLevel;
    std::string name;
    std::string tag;
};

const std::string GetPlayerInfo(std::string source, std::string key);

Clan GetPlayerClanInfo(std::string source);