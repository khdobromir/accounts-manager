#include "accounts_model.hpp"
#include "config.hpp"
#include "omarchy_theme.hpp"

#include <QDir>
#include <QGuiApplication>
#include <QLockFile>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QStandardPaths>
#include <cstdio>

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName("AccountsManager");
    QCoreApplication::setOrganizationName("AccountsManager");
    QCoreApplication::setApplicationVersion("0.1.0");
    QQuickStyle::setStyle("Material");

    const auto directory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (directory.isEmpty() || !QDir().mkpath(directory)) {
        std::fputs("Cannot create application data directory\n", stderr);
        return 1;
    }
    QLockFile lock(directory + "/accounts.lock");
    if (!lock.tryLock(0)) {
        std::fputs("AccountsManager is already running or the data directory is unavailable\n", stderr);
        return 1;
    }
    OmarchyTheme theme;
    AccountsModel accounts(directory + "/accounts.sqlite", ReadApiKey());
    QQmlApplicationEngine engine;
    engine.setInitialProperties({{"accounts", QVariant::fromValue(&accounts)}, {"theme", QVariant::fromValue(&theme)}});
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule("AccountsManager", "Main");
    return app.exec();
}
