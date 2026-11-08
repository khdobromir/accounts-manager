#pragma once

#include <string>

class Player {
protected:
    std::string tag_;
    std::string name_;
    std::string clan_tag_;
    std::string clan_name_;

public:
    Player(std::string tag);

    const std::string& tag() const;
    const std::string& name() const;
    const std::string& clan_tag() const;
    const std::string& clan_name() const;
};