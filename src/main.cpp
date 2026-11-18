#include "accounts_model.hpp"
#include "config.hpp"
#include "desktop_session.hpp"
#include "omarchy_theme.hpp"

#include <QDir>
#include <QGuiApplication>
#include <QLockFile>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QWindow>
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
    const bool background = app.arguments().contains("--background");
    DesktopSession desktop;
    if (background && !desktop.enabled()) return 0;
    QLockFile lock(directory + "/accounts.lock");
    if (!lock.tryLock(0)) {
        if (lock.error() == QLockFile::LockFailedError && (background || DesktopSession::activateRunning())) return 0;
        std::fputs("AccountsManager is already running or the data directory is unavailable\n", stderr);
        return 1;
    }
    const bool registered = desktop.registerActivation();
    if (background && !registered) return 1;
    auto updateQuitPolicy = [&] { app.setQuitOnLastWindowClosed(!registered || !desktop.enabled()); };
    updateQuitPolicy();
    QObject::connect(&desktop, &DesktopSession::changed, &app, updateQuitPolicy);
    OmarchyTheme theme;
    AccountsModel accounts(directory + "/accounts.sqlite", ReadApiKey(background));
    desktop.monitor(accounts);
    QQmlApplicationEngine engine;
    engine.setInitialProperties({{"desktop", QVariant::fromValue(&desktop)}, {"startHidden", background}, {"accounts", QVariant::fromValue(&accounts)}, {"theme", QVariant::fromValue(&theme)}});
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    QObject::connect(&desktop, &DesktopSession::activationRequested, &engine, [&engine] {
        if (engine.rootObjects().isEmpty()) return;
        if (auto* window = qobject_cast<QWindow*>(engine.rootObjects().first())) {
            window->show();
            window->raise();
            window->requestActivate();
        }
    }, Qt::QueuedConnection);
    engine.loadFromModule("AccountsManager", "Main");
    return app.exec();
}
