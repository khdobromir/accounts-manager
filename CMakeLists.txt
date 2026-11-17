cmake_minimum_required(VERSION 4.4.5)

project(
    accounts-manager
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Network Sql Quick QuickControls2)
qt_standard_project_setup(REQUIRES 6.8)
find_package(nlohmann_json CONFIG REQUIRED)
find_path(DOTENV_INCLUDE_DIR
    NAMES dotenv/dotenv.hpp
    REQUIRED
)

qt_add_executable(accounts-manager
    src/main.cpp
    src/config.cpp
    src/classes.cpp
    src/json.cpp
    src/domain.cpp
    src/api_client.cpp
    src/api_client.hpp
    src/local_store.cpp
    src/local_store.hpp
    src/accounts_model.cpp
    src/accounts_model.hpp
    src/omarchy_theme.cpp
    src/omarchy_theme.hpp
)

set(QML_FILES
    qml/AccountsManager/Main.qml
    qml/AccountsManager/AppToolbar.qml
    qml/AccountsManager/Dashboard.qml
    qml/AccountsManager/AccountCard.qml
    qml/AccountsManager/ActivityStatus.qml
    qml/AccountsManager/AccountsPage.qml
    qml/AccountsManager/AccountEditor.qml
    qml/AccountsManager/SettingsDialog.qml
)
foreach(qml_file IN LISTS QML_FILES)
    get_filename_component(qml_name "${qml_file}" NAME)
    set_source_files_properties("${qml_file}" PROPERTIES QT_RESOURCE_ALIAS "${qml_name}")
endforeach()
qt_add_qml_module(accounts-manager
    URI AccountsManager
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/AccountsManager"
    QML_FILES ${QML_FILES}
)

target_link_libraries(accounts-manager PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Sql
    Qt6::Quick
    Qt6::QuickControls2
    nlohmann_json::nlohmann_json
)
target_include_directories(accounts-manager PRIVATE src ${DOTENV_INCLUDE_DIR})
