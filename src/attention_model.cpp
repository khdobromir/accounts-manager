#include "attention_model.hpp"

#include <QDateTime>
#include <QVariantMap>

AttentionModel::AttentionModel(QObject* parent) : QSortFilterProxyModel(parent) {
    setFilterRole(Qt::UserRole + 1);
    setSortRole(filterRole());
    sort(0);
}

bool AttentionModel::filterAcceptsRow(int row, const QModelIndex& parent) const {
    return sourceModel()->index(row, 0, parent).data(filterRole()).toMap().value("attention").toBool();
}

bool AttentionModel::lessThan(const QModelIndex& left, const QModelIndex& right) const {
    const auto a = left.data(sortRole()).toMap().value("deadline").toDateTime();
    const auto b = right.data(sortRole()).toMap().value("deadline").toDateTime();
    if (a == b) return left.row() < right.row();
    return a.isValid() && (!b.isValid() || a < b);
}
