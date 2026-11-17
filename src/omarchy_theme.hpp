#pragma once

#include <QColor>
#include <QFileSystemWatcher>
#include <QObject>
#include <QTimer>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

class OmarchyTheme : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by the application")
    Q_PROPERTY(QColor background READ background NOTIFY changed FINAL)
    Q_PROPERTY(QColor foreground READ foreground NOTIFY changed FINAL)
    Q_PROPERTY(QColor accent READ accent NOTIFY changed FINAL)
    Q_PROPERTY(QColor selectionForeground READ selectionForeground NOTIFY changed FINAL)
    Q_PROPERTY(QVariantMap colors READ colors NOTIFY changed FINAL)
    Q_PROPERTY(bool light READ light NOTIFY changed FINAL)
public:
    explicit OmarchyTheme(const QString& currentDirectory = {}, QObject* parent = nullptr);
    QColor background() const;
    QColor foreground() const;
    QColor accent() const;
    QColor selectionForeground() const;
    QVariantMap colors() const;
    bool light() const;

signals:
    void changed();

private:
    void reload();
    QString directory_;
    QVariantMap colors_;
    bool light_ = false;
    QFileSystemWatcher watcher_;
    QTimer debounce_;
};
