#pragma once

#include "domain.hpp"

#include <QNetworkAccessManager>
#include <QObject>
#include <QQueue>
#include <QTimer>

class ApiClient : public QObject {
    Q_OBJECT
public:
    explicit ApiClient(QByteArray key, QObject* parent = nullptr);
    ~ApiClient() override;
    void fetch(const QString& resource, ResourceKind kind, quint64 generation);
    void cancel();
    bool hasKey() const;

signals:
    void finished(const QString& resource, quint64 generation, const ApiResult& result);

private:
    struct Request { QString resource; ResourceKind kind; quint64 generation; };
    void pump();
    QByteArray key_;
    QNetworkAccessManager network_;
    QQueue<Request> queue_;
    QSet<QNetworkReply*> active_;
    QTimer cooldown_;
    int backoff_seconds_ = 60;
};
