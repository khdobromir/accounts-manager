#include "config.hpp"

#include <dotenv/dotenv.hpp>

#include <exception>

QByteArray ReadApiKey() {
    auto key = qgetenv("API_KEY").trimmed();
    if (key.isEmpty()) {
        try {
            dotenv::load(".env");
            key = qgetenv("API_KEY").trimmed();
        } catch (const std::exception&) {
            return {};
        }
    }
    if (key.contains('\r') || key.contains('\n')) return {};
    return key;
}
