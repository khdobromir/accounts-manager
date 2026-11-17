#include "local_store.hpp"

#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QVariant>

#include <stdexcept>

namespace {
void Execute(QSqlQuery& query, const QString& sql = {}) {
    if (!(sql.isEmpty() ? query.exec() : query.exec(sql)))
        throw std::runtime_error(query.lastError().text().toStdString());
}
void Require(bool success, const QSqlDatabase& db) {
    if (!success) throw std::runtime_error(db.lastError().text().toStdString());
}
QString NonNullText(const QString& text) { return text.isNull() ? QStringLiteral("") : text; }
QString Timestamp(const QDateTime& time) { return time.isValid() ? time.toUTC().toString(Qt::ISODateWithMs) : QStringLiteral(""); }
}

LocalStore::LocalStore(QString path) : path_(std::move(path)) {}
LocalStore::~LocalStore() { close(); }

void LocalStore::load() {
    try {
        if (!QDir().mkpath(QFileInfo(path_).absolutePath())) throw std::runtime_error("Не удалось создать каталог данных");
        db_ = QSqlDatabase::addDatabase("QSQLITE", QUuid::createUuid().toString(QUuid::WithoutBraces));
        db_.setDatabaseName(path_);
        db_.setConnectOptions("QSQLITE_BUSY_TIMEOUT=1000");
        Require(db_.open(), db_);
        QSqlQuery query(db_);
        Execute(query, "PRAGMA user_version");
        if (!query.next()) throw std::runtime_error("Не удалось прочитать версию базы");
        const auto version = query.value(0).toInt();
        query.finish();
        if (version > 1) throw std::runtime_error("База создана более новой версией приложения");
        if (version == 0) {
            Require(db_.transaction(), db_);
            Execute(query, "CREATE TABLE accounts (tag TEXT PRIMARY KEY, alias TEXT NOT NULL, position INTEGER NOT NULL, "
                           "war_clan TEXT NOT NULL, cwl_clan TEXT NOT NULL, cwl_season TEXT NOT NULL, "
                           "transfer_clan TEXT NOT NULL, transfer_at TEXT NOT NULL)");
            Execute(query, "CREATE TABLE settings (key TEXT PRIMARY KEY, value INTEGER NOT NULL)");
            Execute(query, "CREATE TABLE api_cache (resource TEXT PRIMARY KEY, body BLOB NOT NULL, fetched_at TEXT NOT NULL)");
            Execute(query, "PRAGMA user_version=1");
            Require(db_.commit(), db_);
        }
        StoredState state;
        Execute(query, "SELECT tag, alias, war_clan, cwl_clan, cwl_season, transfer_clan, transfer_at FROM accounts ORDER BY position, tag");
        QSet<QString> tags;
        while (query.next()) {
            AccountSettings account;
            account.tag = NormalizeTag(query.value(0).toString());
            if (tags.contains(account.tag)) throw std::runtime_error("В базе повторяется тег аккаунта");
            tags.insert(account.tag);
            account.alias = query.value(1).toString();
            account.war_clan = query.value(2).toString();
            account.cwl_clan = query.value(3).toString();
            account.cwl_season = query.value(4).toString();
            account.transfer_clan = query.value(5).toString();
            for (const auto& tag : {account.war_clan, account.cwl_clan, account.transfer_clan})
                if (!tag.isEmpty()) NormalizeTag(tag);
            account.transfer_at = QDateTime::fromString(query.value(6).toString(), Qt::ISODateWithMs);
            state.accounts.append(account);
        }
        Execute(query, "SELECT value FROM settings WHERE key='refresh_seconds'");
        if (query.next()) {
            state.refresh_seconds = query.value(0).toInt();
            if (state.refresh_seconds != 0 && (state.refresh_seconds < 60 || state.refresh_seconds > 86400))
                throw std::runtime_error("Неверный период обновления в базе");
        }
        Execute(query, "SELECT resource, body, fetched_at FROM api_cache");
        while (query.next()) {
            const auto at = QDateTime::fromString(query.value(2).toString(), Qt::ISODateWithMs);
            if (at.isValid()) state.cache.insert(query.value(0).toString(), {query.value(1).toByteArray(), at.toUTC()});
        }
        query.finish();
        ready_ = true;
        emit loaded(state, {});
    } catch (const std::exception& error) {
        if (db_.isOpen()) db_.rollback();
        emit loaded({}, QString::fromUtf8(error.what()));
    }
}

void LocalStore::saveAccounts(const QList<AccountSettings>& accounts) {
    try {
        if (!ready_) throw std::runtime_error("Хранилище недоступно");
        Require(db_.transaction(), db_);
        QSqlQuery query(db_);
        Execute(query, "DELETE FROM accounts");
        if (!query.prepare("INSERT INTO accounts VALUES (?, ?, ?, ?, ?, ?, ?, ?)"))
            throw std::runtime_error(query.lastError().text().toStdString());
        int position = 0;
        for (const auto& account : accounts) {
            query.bindValue(0, account.tag);
            query.bindValue(1, NonNullText(account.alias));
            query.bindValue(2, position++);
            query.bindValue(3, NonNullText(account.war_clan));
            query.bindValue(4, NonNullText(account.cwl_clan));
            query.bindValue(5, NonNullText(account.cwl_season));
            query.bindValue(6, NonNullText(account.transfer_clan));
            query.bindValue(7, Timestamp(account.transfer_at));
            Execute(query);
        }
        query.finish();
        Require(db_.commit(), db_);
        emit accountsSaved(accounts, {});
    } catch (const std::exception& error) {
        db_.rollback();
        emit accountsSaved(accounts, QString::fromUtf8(error.what()));
    }
}

void LocalStore::saveCache(const QString& key, const QByteArray& body, const QDateTime& fetched) {
    if (!ready_) return;
    QSqlQuery query(db_);
    query.prepare("INSERT INTO api_cache VALUES (?, ?, ?) ON CONFLICT(resource) DO UPDATE SET body=excluded.body, fetched_at=excluded.fetched_at");
    query.addBindValue(key);
    query.addBindValue(body);
    query.addBindValue(Timestamp(fetched));
    if (!query.exec()) emit failed(tr("Не удалось сохранить кеш: %1").arg(query.lastError().text()));
}

void LocalStore::pruneCache(const QStringList& keep) {
    if (!ready_) return;
    try {
        Require(db_.transaction(), db_);
        const QSet<QString> retained(keep.begin(), keep.end());
        QSqlQuery select(db_);
        Execute(select, "SELECT resource FROM api_cache");
        QStringList obsolete;
        while (select.next()) if (!retained.contains(select.value(0).toString())) obsolete.append(select.value(0).toString());
        select.finish();
        QSqlQuery remove(db_);
        if (!remove.prepare("DELETE FROM api_cache WHERE resource=?")) throw std::runtime_error(remove.lastError().text().toStdString());
        for (const auto& key : obsolete) { remove.bindValue(0, key); Execute(remove); }
        remove.finish();
        Require(db_.commit(), db_);
    } catch (const std::exception& error) {
        db_.rollback();
        emit failed(tr("Не удалось очистить неиспользуемый кеш: %1").arg(QString::fromUtf8(error.what())));
    }
}

void LocalStore::setInterval(int seconds) {
    if (!ready_ || (seconds != 0 && (seconds < 60 || seconds > 86400))) {
        emit intervalSaved(seconds, tr("Недопустимый период обновления или хранилище недоступно"));
        return;
    }
    QSqlQuery query(db_);
    query.prepare("INSERT INTO settings VALUES ('refresh_seconds', ?) ON CONFLICT(key) DO UPDATE SET value=excluded.value");
    query.addBindValue(seconds);
    emit intervalSaved(seconds, query.exec() ? QString{} : query.lastError().text());
}

void LocalStore::close() {
    ready_ = false;
    if (!db_.isValid()) return;
    const auto name = db_.connectionName();
    db_.close();
    db_ = QSqlDatabase{};
    QSqlDatabase::removeDatabase(name);
}

void LocalStore::setCwlSeason(const QString& tag, const QString& clan, const QString& season) {
    if (!ready_) return;
    QSqlQuery query(db_);
    query.prepare("UPDATE accounts SET cwl_season=? WHERE tag=? AND cwl_clan=?");
    query.addBindValue(season);
    query.addBindValue(tag);
    query.addBindValue(clan);
    if (!query.exec()) emit failed(tr("Не удалось сохранить сезон ЛВК: %1").arg(query.lastError().text()));
}
