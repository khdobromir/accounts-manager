#pragma once

#include "api_client.hpp"
#include "local_store.hpp"

#include <QAbstractListModel>
#include <QThread>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

class AccountsModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by the application")
    Q_PROPERTY(bool ready READ ready NOTIFY stateChanged)
    Q_PROPERTY(bool saving READ saving NOTIFY stateChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY stateChanged)
    Q_PROPERTY(int count READ count NOTIFY stateChanged)
    Q_PROPERTY(int refreshMinutes READ refreshMinutes NOTIFY stateChanged)
    Q_PROPERTY(QString error READ error NOTIFY stateChanged)
    Q_PROPERTY(QString databasePath READ databasePath CONSTANT)
public:
    AccountsModel(QString databasePath, QByteArray apiKey, QObject* parent = nullptr);
    ~AccountsModel() override;
    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    bool ready() const;
    bool saving() const;
    bool busy() const;
    int count() const;
    int refreshMinutes() const;
    QString error() const;
    QString databasePath() const;
    bool hasReminder(const QString& event) const { return reminders_.contains(event); }
    void recordReminder(const QString& event);
    Q_INVOKABLE bool addAccount(const QString& tag);
    Q_INVOKABLE bool updateAccount(const QString& tag, const QString& alias, const QString& warClan,
                                   const QString& cwlClan, const QString& transferClan, const QString& transferAt);
    Q_INVOKABLE void removeAccount(const QString& tag);
    Q_INVOKABLE void moveAccount(const QString& tag, int direction);
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setRefreshMinutes(int minutes);
    Q_INVOKABLE QVariantMap accountSettings(const QString& tag) const;

signals:
    void stateChanged();
    void accountsCommitted();
    void intervalCommitted();

private:
    struct Resource {
        std::optional<ApiData> data = std::nullopt;
        QDateTime fetched;
        QString error;
        QDateTime group_not_found_at;
        bool loading = false;
        bool from_cache = true;
    };
    template<typename T> const T* value(const QString& key) const {
        const auto it = resources_.constFind(key);
        return it != resources_.cend() && it->data ? std::get_if<T>(&*it->data) : nullptr;
    }
    void request(const QString& key, ResourceKind kind);
    void schedule();
    void received(const QString& key, quint64 generation, const ApiResult& result);
    void saveAccounts(const QList<AccountSettings>& accounts);
    void notifyRows();
    int findAccount(const QString& tag) const;
    QVector<RoundWar> roundWars(const CwlGroup& group, QStringList* dependencies = nullptr) const;
    QVariantMap activityMap(Activity activity, const QStringList& dependencies, bool regularWar = false) const;
    QVariantMap row(const AccountSettings& account) const;

    QString database_path_;
    ApiClient api_;
    QThread storage_thread_;
    LocalStore* store_;
    QList<AccountSettings> accounts_;
    QHash<QString, Resource> resources_;
    QSet<QString> requested_;
    QSet<QString> reminders_;
    QTimer refresh_timer_;
    QTimer clock_timer_;
    quint64 generation_ = 0;
    int pending_ = 0;
    int refresh_seconds_ = 300;
    bool ready_ = false;
    bool saving_ = false;
    QString error_;
};
