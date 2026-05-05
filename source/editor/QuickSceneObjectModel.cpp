#include "QuickSceneObjectModel.h"

#include "QuickComponentModel.h"
#include "model/ComponentModel.h"
#include "model/SceneObjectModel.h"

namespace CSEditor
{

QuickSceneObjectModel::QuickSceneObjectModel(SceneObjectModel* model, QObject* parent)
    : QAbstractListModel(parent), m_model(model)
{
    rebuild();
    if (m_model != nullptr) {
        m_model->AddListener([this] { onDomainChanged(); });
    }
}

QuickSceneObjectModel::~QuickSceneObjectModel() = default;

int QuickSceneObjectModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(m_quickComponents.size());
}

QVariant QuickSceneObjectModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    const int row = index.row();
    if (row < 0 || static_cast<size_t>(row) >= m_quickComponents.size()) {
        return {};
    }
    if (role == ComponentRole) {
        return QVariant::fromValue(static_cast<QObject*>(m_quickComponents[row].get()));
    }
    return {};
}

QHash<int, QByteArray> QuickSceneObjectModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ComponentRole] = "component";
    return roles;
}

QString QuickSceneObjectModel::name() const
{
    return (m_model != nullptr) ? QString::fromStdString(m_model->GetName()) : QString{};
}

void QuickSceneObjectModel::setName(const QString& value)
{
    if (m_model == nullptr) {
        return;
    }
    const std::string s = value.toStdString();
    if (m_model->GetName() == s) {
        return;
    }
    m_model->SetName(s);
    // Listener re-entry will emit changed().
}

bool QuickSceneObjectModel::active() const
{
    return (m_model != nullptr) && m_model->IsActive();
}

void QuickSceneObjectModel::setActive(bool value)
{
    if (m_model == nullptr || m_model->IsActive() == value) {
        return;
    }
    m_model->SetActive(value);
}

bool QuickSceneObjectModel::hasSelection() const
{
    return (m_model != nullptr) && m_model->HasSelection();
}

void QuickSceneObjectModel::onDomainChanged()
{
    rebuild();
    emit changed();
}

void QuickSceneObjectModel::rebuild()
{
    beginResetModel();
    m_quickComponents.clear();
    if (m_model != nullptr) {
        const size_t n = m_model->GetComponentCount();
        m_quickComponents.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            m_quickComponents.emplace_back(std::make_unique<QuickComponentModel>(&m_model->GetComponent(i), this));
        }
    }
    endResetModel();
}

} // namespace CSEditor
