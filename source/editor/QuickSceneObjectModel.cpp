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
        // Sync our revision cache with the domain state that `rebuild()`
        // just snapshotted, so the next onDomainChanged() only takes the
        // slow path if a genuine structural change happened afterwards.
        m_lastStructureRevision = m_model->GetStructureRevision();
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
    // If the domain signalled a structural change (selection swapped /
    // SceneObjectModel::Clear), every ComponentModel* we stashed inside our
    // QuickComponentModel adapters is now dangling. We MUST rebuild before
    // emitting any dataChanged; otherwise the next QML role query will
    // dereference freed memory and crash inside PropertyItem::GetValue().
    const size_t domainRevision = (m_model != nullptr) ? m_model->GetStructureRevision() : 0;
    const bool structuralChange = (domainRevision != m_lastStructureRevision);

    // Fast path: when the component count is unchanged AND no structural
    // revision happened we can keep the existing QuickComponentModel row
    // adapters alive and just tell them their data is stale. This avoids
    // a model reset which would blow away QML delegate state (scroll
    // position, focused editor, etc.).
    const size_t domainCount = (m_model != nullptr) ? m_model->GetComponentCount() : 0;
    if (!structuralChange && domainCount == m_quickComponents.size()) {
        for (auto& qc : m_quickComponents) {
            if (qc) {
                qc->notifyAllChanged();
            }
        }
        emit changed();
        return;
    }

    // Slow path: structural change (selection swapped / component added or
    // removed). Rebuild the list.
    rebuild();
    m_lastStructureRevision = domainRevision;
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
            auto adapter = std::make_unique<QuickComponentModel>(&m_model->GetComponent(i), this);
            // Bubble per-property edits up to interested QML listeners. We
            // re-emit `changed()` which already drives name/active/hasSelection
            // bindings; future viewport-side observers (gizmo, dirty flag)
            // can listen to the same signal.
            QObject::connect(adapter.get(), &QAbstractListModel::dataChanged, this,
                             [this](const QModelIndex&, const QModelIndex&, const QList<int>&) { emit changed(); });
            m_quickComponents.emplace_back(std::move(adapter));
        }
    }
    endResetModel();
}

} // namespace CSEditor
