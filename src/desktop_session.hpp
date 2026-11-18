#pragma once

#include <QObject>
#include <QSet>
#include <QtQml/qqmlregistration.h>

class AccountsModel;

class DesktopSession : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by the application")
    Q_CLASSINFO("D-Bus Interface", "org.accountsmanager.Desktop")
    Q_PROPERTY(bool enabled READ enabled NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
public:
    explicit DesktopSession(QObject* parent = nullptr);
    bool enabled() const { return enabled_; }
    QString error() const { return error_; }
    Q_INVOKABLE bool setEnabled(bool enabled);
    void monitor(AccountsModel& model);
    bool registerActivation();
    static bool activateRunning();

public slots:
    Q_SCRIPTABLE void Activate() { emit activationRequested(); }

signals:
    void activationRequested();
    void changed();

private:
    void deliver(AccountsModel& model);
    QString autostart_;
    QString error_;
    QSet<QString> pending_;
    bool enabled_ = false;
    bool registered_ = false;
};
