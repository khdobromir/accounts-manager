#pragma once

#include "domain.hpp"

#include <QObject>
#include <QSqlDatabase>

class LocalStore : public QObject {
    Q_OBJECT
public:
    explicit LocalStore(QString path);
    ~LocalStore() override;
    void load();
    void saveAccounts(const QList<AccountSettings>& accounts);
    void saveCache(const QString& key, const QByteArray& body, const QDateTime& fetched);
    void pruneCache(const QStringList& keep);
    void setInterval(int seconds);
    void setCwlSeason(const QString& tag, const QString& clan, const QString& season);
    void close();
    void recordReminder(const QString& event);

signals:
    void loaded(const StoredState& state, const QString& error);
    void accountsSaved(const QList<AccountSettings>& accounts, const QString& error);
    void intervalSaved(int seconds, const QString& error);
    void failed(const QString& error);

private:
    QString path_;
    QSqlDatabase db_;
    bool ready_ = false;
};
