cmake_minimum_required(VERSION 4.4.5)

project(
    accounts-manager
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_path(DOTENV_INCLUDE_DIR
    NAMES dotenv/dotenv.hpp
    REQUIRED
)

add_executable(accounts-manager
    src/main.cpp
    src/config.cpp
    src/classes.cpp
    src/json.cpp
)

target_link_libraries(accounts-manager PRIVATE
    cpr::cpr
    nlohmann_json::nlohmann_json
)
target_include_directories(accounts-manager PRIVATE ${DOTENV_INCLUDE_DIR})

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(accounts-manager PRIVATE
        -static-libstdc++
        -static-libgcc
    )
endif()