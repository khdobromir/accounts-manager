#pragma once

#include <QSortFilterProxyModel>
#include <QtQml/qqmlregistration.h>

class AttentionModel : public QSortFilterProxyModel {
    Q_OBJECT
    QML_ELEMENT
public:
    explicit AttentionModel(QObject* parent = nullptr);
protected:
    bool filterAcceptsRow(int row, const QModelIndex& parent) const override;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
};
