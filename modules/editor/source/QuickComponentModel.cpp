#include "QuickComponentModel.h"

#include "model/ComponentModel.h"
#include "model/PropertyItem.h"
#include "utils/QVariantsAny.h"

namespace CSEditor
{

QuickComponentModel::QuickComponentModel(ComponentModel* model, QObject* parent)
    : QAbstractListModel(parent), m_model(model)
{}

QuickComponentModel::~QuickComponentModel() = default;

int QuickComponentModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || m_model == nullptr) {
        return 0;
    }
    return static_cast<int>(m_model->GetPropertyCount());
}

QVariant QuickComponentModel::data(const QModelIndex& index, int role) const
{
    if (m_model == nullptr || !index.isValid()) {
        return {};
    }
    const int row = index.row();
    if (row < 0 || static_cast<size_t>(row) >= m_model->GetPropertyCount()) {
        return {};
    }

    const auto& prop = m_model->GetProperty(static_cast<size_t>(row));

    switch (role) {
    case UINameRole:
    case Qt::DisplayRole:
        return QString::fromUtf8(prop.GetUIName().data(), static_cast<int>(prop.GetUIName().size()));
    case TypeTagRole:
        return static_cast<int>(prop.GetType());
    case ValueRole:
        return AnyToQVariant(prop.GetValue());
    case WritableRole:
        return prop.IsWritable();
    default:
        return {};
    }
}

bool QuickComponentModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != ValueRole && role != Qt::EditRole) {
        return false;
    }
    return setValue(index.row(), value);
}

QHash<int, QByteArray> QuickComponentModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[UINameRole] = "uiName";
    roles[TypeTagRole] = "typeTag";
    roles[ValueRole] = "value";
    roles[WritableRole] = "writable";
    return roles;
}

QString QuickComponentModel::componentName() const
{
    if (m_model == nullptr) {
        return {};
    }
    const auto name = m_model->GetTypeName();
    QString full = QString::fromUtf8(name.data(), static_cast<int>(name.size()));
    // Strip any namespace qualifiers for UI display (e.g. "CS::Transform" -> "Transform").
    const int sep = full.lastIndexOf(QStringLiteral("::"));
    if (sep >= 0) {
        full.remove(0, sep + 2);
    }
    return full;
}

bool QuickComponentModel::setValue(int row, const QVariant& value)
{
    if (m_model == nullptr || row < 0 || static_cast<size_t>(row) >= m_model->GetPropertyCount()) {
        return false;
    }
    auto& prop = m_model->GetProperty(static_cast<size_t>(row));
    std::any anyVal = QVariantToAny(value);
    if (!anyVal.has_value()) {
        return false;
    }
    if (!prop.SetValue(anyVal)) {
        return false;
    }
    const QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx, {ValueRole});
    return true;
}

void QuickComponentModel::notifyAllChanged()
{
    if (m_model == nullptr) {
        return;
    }
    const int count = static_cast<int>(m_model->GetPropertyCount());
    if (count == 0) {
        return;
    }
    emit dataChanged(index(0, 0), index(count - 1, 0), {ValueRole, WritableRole, UINameRole, TypeTagRole});
}

} // namespace CSEditor
