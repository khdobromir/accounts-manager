#include "config.hpp"

#include <dotenv/dotenv.hpp>

#include <string>

const std::string GetEnv(std::string name) {
    dotenv::load(".env");
    auto var = dotenv::get<std::string>(name);
    if (!var || var->empty()) {
        throw std::runtime_error("error in src/config.cpp/GetEnv: failed to get API key");
    }
    return *var;
}