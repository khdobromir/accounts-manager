#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "config.hpp"

#include <print>

using json = nlohmann::json;

std::string API_KEY = GetEnv("API_KEY");

std::string GetName(std::string text) {
    try {
        json data = json::parse(text);
        return data.value("name", std::string{});
    } catch (const json::exception& e) {
        std::printf("GetName: JSON parsing error: {}\n", e.what());
        return "";
    }
}

int main() {
    auto response = cpr::Get(
        cpr::Url{"https://api.clashofclans.com/v1/players/%23Q2UV9VJ8J"},
        cpr::Bearer{API_KEY},
        cpr::Timeout{10000}
    );

    std::println("HTTP status: {}", response.status_code);

    std::string name = GetName(response.text);
    std:println("player's name: {}", name);
}
