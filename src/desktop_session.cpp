#include "desktop_session.hpp"
#include "accounts_model.hpp"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDir>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>
#include <QThread>
#include <QTimer>

namespace {
constexpr auto Service = "org.accountsmanager.Desktop";

QByteArray AutostartEntry() {
    auto path = QCoreApplication::applicationFilePath();
    if (path.contains('\n') || path.contains('\r')) return {};
    // Escape the quoted Exec argument, then the desktop-entry string.
    path.replace("\\", "\\\\").replace("\"", "\\\"").replace("$", "\\$").replace("`", "\\`");
    path.replace("\\", "\\\\").replace("%", "%%");
    return "[Desktop Entry]\nType=Application\nName=AccountsManager\nExec=\"" + path.toUtf8() + "\" --background\n";
}
}

DesktopSession::DesktopSession(QObject* parent) : QObject(parent),
    autostart_(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
               + "/autostart/org.accountsmanager.Desktop.desktop"), enabled_(QFileInfo::exists(autostart_)) {}

bool DesktopSession::registerActivation() {
    auto bus = QDBusConnection::sessionBus();
    registered_ = bus.registerObject("/Desktop", this, QDBusConnection::ExportScriptableSlots)
        && bus.registerService(Service);
    return registered_;
}

bool DesktopSession::activateRunning() {
    const auto message = QDBusMessage::createMethodCall(Service, "/Desktop", Service, "Activate");
    const auto bus = QDBusConnection::sessionBus();
    // The first process acquires the database lock before it owns the bus name.
    for (int attempt = 0; attempt < 20; ++attempt) {
        const auto reply = bus.call(message, QDBus::Block, 3000);
        if (reply.type() == QDBusMessage::ReplyMessage) return true;
        if (reply.errorName() != "org.freedesktop.DBus.Error.ServiceUnknown") break;
        QThread::msleep(50);
    }
    return false;
}

bool DesktopSession::setEnabled(bool enabled) {
    QString error;
    if (enabled) {
        QSaveFile file(autostart_);
        const auto contents = AutostartEntry();
        if (!registered_) error = tr("Недоступна сеансовая шина D-Bus");
        else if (contents.isEmpty() || !QDir().mkpath(QFileInfo(file.fileName()).absolutePath()) ||
                 !file.open(QIODevice::WriteOnly) || file.write(contents) != contents.size() || !file.commit())
            error = tr("Не удалось сохранить запись автозапуска");
    } else if (QFileInfo::exists(autostart_) && !QFile::remove(autostart_)) {
        error = tr("Не удалось отключить автозапуск");
    }
    error_ = error;
    if (error_.isEmpty()) enabled_ = enabled;
    emit changed();
    return error_.isEmpty();
}

void DesktopSession::monitor(AccountsModel& model) {
    auto* timer = new QTimer(&model);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this, &model] { deliver(model); });
    auto schedule = [timer] { timer->start(0); };
    connect(&model, &AccountsModel::stateChanged, this, schedule);
    connect(this, &DesktopSession::changed, timer, schedule);
    schedule();
}

void DesktopSession::deliver(AccountsModel& model) {
    if (!enabled_ || !model.ready() || model.saving()) return;
    const auto now = QDateTime::currentDateTimeUtc();
    for (int i = 0; i < model.rowCount(); ++i) {
        const auto row = model.index(i).data(Qt::UserRole + 1).toMap();
        const auto tag = row.value("tag").toString();
        const auto name = row.value("name").toString();
        auto send = [&](const QString& kind, const QString& period, const QString& body) {
            const auto event = tag + "/" + kind + "/" + period;
            if (model.hasReminder(event) || pending_.contains(event)) return;
            auto message = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "Notify");
            message.setArguments({"AccountsManager", 0u, QString{}, name.toHtmlEscaped(), body.toHtmlEscaped(),
                                  QStringList{}, QVariantMap{}, -1});
            pending_.insert(event);
            auto* reply = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message, 5000), this);
            connect(reply, &QDBusPendingCallWatcher::finished, &model, [this, &model, event, reply] {
                const QDBusPendingReply<uint> result = *reply;
                pending_.remove(event);
                if (!result.isError()) model.recordReminder(event);
                reply->deleteLater();
            });
        };
        for (const auto& kind : {QStringLiteral("war"), QStringLiteral("cwl")}) {
            const auto mode = row.value(kind).toMap();
            const auto end = mode.value("endUtc").toDateTime();
            if (!mode.value("attention").toBool() || end <= now || end > now.addSecs(3600)) continue;
            const auto clan = row.value(kind == "war" ? "participatingWarClan" : "cwlClan").toString();
            const auto label = kind == "war" ? tr("КВ") : tr("ЛВК");
            send(kind, clan + "/" + end.toString(Qt::ISODateWithMs),
                 tr("%1 · %2 · Остались атаки до %3").arg(label, clan, end.toLocalTime().toString("HH:mm")));
        }
        const auto transfer = row.value("transferAt").toDateTime();
        if (transfer.isValid() && transfer <= now) {
            const auto target = row.value("transferClan").toString();
            send("transfer", target + "/" + transfer.toString(Qt::ISODateWithMs), tr("Пора перейти в %1").arg(target));
        }
    }
}
