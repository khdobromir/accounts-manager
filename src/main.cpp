#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "config.hpp"
#include "classes.hpp"

#include <print>

using json = nlohmann::json;

std::string GetName(std::string text) {
    try {
        std::string API_KEY = GetEnv("API_KEY");
        json data = json::parse(text);
        return data.value("name", std::string{});
    } catch (const json::exception& e) {
        std::println("GetName: JSON parsing error: {}", e.what());
        return "";
    }
}

int main() {
    Player player1("#Q2UV9VJ8J");
    std::println("player's name: {}", player1.name());
}
