#include "api_client.hpp"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include <algorithm>
#include <exception>
#include <utility>

ApiClient::ApiClient(QByteArray key, QObject* parent)
    : QObject(parent), key_(std::move(key)) {
    cooldown_.setSingleShot(true);
    connect(&cooldown_, &QTimer::timeout, this, &ApiClient::pump);
    connect(&network_, &QNetworkAccessManager::sslErrors, this, [](QNetworkReply* reply, const QList<QSslError>&) {
        reply->setProperty("tlsRejected", true);
        reply->abort();
    });
}

ApiClient::~ApiClient() {
    for (auto* reply : std::as_const(active_)) {
        disconnect(reply, nullptr, this, nullptr);
        reply->abort();
    }
}

bool ApiClient::hasKey() const { return !key_.isEmpty(); }

void ApiClient::fetch(const QString& resource, ResourceKind kind, quint64 generation) {
    if (!hasKey() || cooldown_.isActive()) {
        const auto error = !hasKey() ? tr("Не задан API_KEY; показаны сохранённые данные")
                                    : tr("Пауза после ограничения запросов API (429)");
        QTimer::singleShot(0, this, [this, resource, generation, error] {
            emit finished(resource, generation, ApiResult{.error = error});
        });
        return;
    }
    queue_.enqueue({resource, kind, generation});
    pump();
}

void ApiClient::cancel() {
    queue_.clear();
    const auto active = active_;
    for (auto* reply : active) reply->abort();
}

void ApiClient::pump() {
    while (!queue_.isEmpty() && active_.size() < 4 && !cooldown_.isActive()) {
        const auto request = queue_.dequeue();
        QNetworkRequest http(QUrl::fromEncoded("https://api.clashofclans.com/v1" + request.resource.toUtf8()));
        http.setRawHeader("Authorization", "Bearer " + key_);
        http.setRawHeader("Accept", "application/json");
        http.setTransferTimeout(10000);
        http.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
        auto* reply = network_.get(http);
        active_.insert(reply);
        auto* deadline = new QTimer(reply);
        deadline->setSingleShot(true);
        connect(deadline, &QTimer::timeout, reply, [reply] {
            reply->setProperty("deadlineExceeded", true);
            reply->abort();
        });
        deadline->start(10000);
        // Keep a bounded response even if a server or proxy returns an unexpected document.
        connect(reply, &QNetworkReply::readyRead, reply, [reply] {
            if (reply->bytesAvailable() > 4 * 1024 * 1024) {
                reply->setProperty("oversized", true);
                reply->abort();
            }
        });
        connect(reply, &QNetworkReply::finished, this, [this, reply, deadline, request] {
            deadline->stop();
            active_.remove(reply);
            ApiResult result;
            result.http_status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (result.http_status == 429) {
                bool ok = false;
                auto seconds = reply->rawHeader("Retry-After").toLongLong(&ok);
                if (!ok) {
                    const auto at = QDateTime::fromString(QString::fromLatin1(reply->rawHeader("Retry-After")), Qt::RFC2822Date);
                    if (at.isValid()) { seconds = QDateTime::currentDateTimeUtc().secsTo(at); ok = true; }
                }
                const auto delay = ok ? std::clamp<qint64>(seconds, 1, 86400) : backoff_seconds_;
                backoff_seconds_ = (std::min)(backoff_seconds_ * 2, 3600);
                cooldown_.start((std::max)(cooldown_.remainingTime(), static_cast<int>(delay * 1000)));
                result.error = tr("API ограничил запросы (429); пауза %1 с").arg(delay);
            } else if (reply->property("oversized").toBool()) {
                result.error = tr("Ответ API превышает допустимый размер");
            } else if (reply->property("deadlineExceeded").toBool()) {
                result.error = tr("Время запроса истекло (10 с)");
            } else if (reply->property("tlsRejected").toBool()) {
                result.error = tr("Не удалось проверить TLS-сертификат сервера");
            } else if (result.http_status && result.http_status != 200) {
                result.error = tr("Ошибка API: HTTP %1").arg(result.http_status);
            } else if (reply->error() != QNetworkReply::NoError) {
                result.error = tr("Сетевая ошибка (%1)").arg(static_cast<int>(reply->error()));
            } else {
                try {
                    result.body = reply->readAll();
                    result.data = ParseResource(request.kind, result.body);
                    if (!ResourceMatches(request.resource, *result.data)) throw std::runtime_error("Mismatched resource tag");
                    result.fetched = QDateTime::currentDateTimeUtc();
                    if (!cooldown_.isActive()) backoff_seconds_ = 60;
                } catch (const std::exception&) {
                    result.data.reset();
                    result.body.clear();
                    result.error = tr("Ответ API не соответствует ожидаемому формату");
                }
            }
            reply->deleteLater();
            emit finished(request.resource, request.generation, result);
            if (cooldown_.isActive()) {
                const auto waiting = std::exchange(queue_, {});
                for (const auto& next : waiting)
                    emit finished(next.resource, next.generation, ApiResult{.error = tr("Пауза после ограничения запросов API (429)")});
            }
            pump();
        });
    }
}
