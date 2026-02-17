#include "QuickTreeModel.h"
#include "model/TreeModel.h"
#include "utils/QVariantsAny.h"

namespace CSEditor
{

static inline const TreeNode* getTreeNode(const TreeModel* model, const QModelIndex& index)
{
    if (!index.isValid() && model != nullptr) {
        return model->GetRoot();
    }
    return reinterpret_cast<TreeNode*>(index.internalPointer());
}

QuickTreeModel::QuickTreeModel(TreeModel* model, QObject* parent) : m_model(model), QAbstractItemModel(parent)
{
    if (m_model != nullptr) {
        initialize();
    }
}

QuickTreeModel::QuickTreeModel(QObject* parent) : QuickTreeModel(nullptr, parent) {}

QuickTreeModel::~QuickTreeModel()
{
    // 根节点自动析构，递归删除所有子节点
}

QVariant QuickTreeModel::data(const QModelIndex& index, int role) const
{
    auto node = getTreeNode(m_model, index);
    if (node == nullptr) {
        return {};
    }
    if (role == Qt::DisplayRole) {
        // 假设 TreeModel 的第一个属性是节点名称（适配 display 角色）
        auto nameProp = "name";
        auto nameValue = node->GetProperty(nameProp);
        return AnyToQVariant(nameValue);
    }
    auto roleName = getRoleName(role);
    auto property = node->GetProperty(roleName);
    return AnyToQVariant(property);
}

QVariant QuickTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return (orientation == Qt::Horizontal && role == Qt::DisplayRole) ? QVariant::fromValue(1000) : QVariant{};
}

bool QuickTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    auto constNode = getTreeNode(m_model, index);
    auto node = m_model->GetEditableNode(constNode);
    if (node == nullptr) {
        return false;
    }
    if (role > Qt::UserRole) {
        auto roleName = getRoleName(role);
        auto property = QVariantToAny(value);
        node->SetProperty(roleName, property);
    }

    return true;
}

QModelIndex QuickTreeModel::parent(const QModelIndex& index) const
{
    auto node = getTreeNode(m_model, index);

    if (node == nullptr || node == m_model->GetRoot()) {
        return QModelIndex();
    }

    auto parent = node->GetParent();
    if (parent == nullptr || parent == m_model->GetRoot()) {
        return QModelIndex();
    }

    return createIndex(IndexOfChildInParent(parent), 0, parent);
}

int QuickTreeModel::rowCount(const QModelIndex& parent) const
{
    auto parentNode = getTreeNode(m_model, parent);
    auto count = parentNode != nullptr ? parentNode->GetChildCount() : 0;
    return count;
}

int QuickTreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QModelIndex QuickTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent) || m_model == nullptr) {
        return QModelIndex{};
    }

    auto parentNode = getTreeNode(m_model, parent);

    if (parentNode == nullptr || row >= parentNode->GetChildCount())
        return QModelIndex{};

    auto childNode = parentNode->GetChild(row);

    auto result = createIndex(row, column, childNode);

    return result;
}

QHash<int, QByteArray> QuickTreeModel::roleNames() const
{
    return m_roleNames;
}

QModelIndex QuickTreeModel::addNode(const QString& nodeName, bool active, const QModelIndex& parentIndex)
{
    beginInsertRows(parentIndex, 0, 0);

    endInsertRows();

    return createIndex(0, 0, nullptr);
}

bool QuickTreeModel::removeNode(const QModelIndex& index)
{
    if (!index.isValid())
        return false;

    auto row = index.row();

    beginRemoveRows(parent(index), row, row);

    endRemoveRows();

    return true;
}

bool QuickTreeModel::toggleNodeActive(const QModelIndex& index)
{
    if (!index.isValid())
        return false;

    return true;
}

bool QuickTreeModel::toggleNodeExpanded(const QModelIndex& index)
{
    if (!index.isValid())
        return false;

    return true;
}

void QuickTreeModel::clear()
{
    beginResetModel();

    endResetModel();
}

void QuickTreeModel::setModel(TreeModel* model)
{
    // clear();
    beginResetModel();
    m_model = model;

    initialize();

    endResetModel();
}

void QuickTreeModel::initialize()
{
    assert(m_model != nullptr);

    m_roleNames = QAbstractItemModel::roleNames();

    auto properties = m_model->GetPropertyNames();
    for (int index = 0; index < properties.size(); ++index) {
        QByteArray bytes = QByteArray::fromStdString(properties[index].data());
        m_roleNames.insert(Qt::UserRole + index + 1, bytes);
    }

    // constructNode(QModelIndex(), m_model->GetRoot());
}

std::string QuickTreeModel::getRoleName(int role) const
{
    if (auto it = m_roleNames.find(role); it != m_roleNames.end()) {
        return it->toStdString();
    }
    return std::string{};
}

void QuickTreeModel::constructNode(const QModelIndex& parent, const TreeNode* parentNode)
{
    for (uint32_t row = 0u; row < parentNode->GetChildCount(); ++row) {
        auto childNode = parentNode->GetChild(row);
        auto child = index(row, 0, parent);
        constructNode(child, childNode);
    }
}

} // namespace CSEditor
