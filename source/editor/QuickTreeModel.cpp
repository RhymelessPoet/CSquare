#include "QuickTreeModel.h"

namespace CSEditor
{

QuickTreeModel::QuickTreeModel(QObject* parent) : QAbstractItemModel(parent)
{
    // 初始化根节点
}

QuickTreeModel::~QuickTreeModel()
{
    // 根节点自动析构，递归删除所有子节点
}

QVariant QuickTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();
}

bool QuickTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
        return false;

    return false;
}

QModelIndex QuickTreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    return createIndex(0, 0, nullptr);
}

int QuickTreeModel::rowCount(const QModelIndex& parent) const
{
    return 0;
}

int QuickTreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    // 固定为 1 列（适配 TreeView 单列展示）
    return 1;
}

QModelIndex QuickTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (column != 0 || row < 0)
        return QModelIndex();

    return QModelIndex();
}

QHash<int, QByteArray> QuickTreeModel::roleNames() const
{
    return m_roleNames;
}

QModelIndex QuickTreeModel::rootIndex() const
{
    return createIndex(0, 0, nullptr);
}

QModelIndex QuickTreeModel::addNode(const QString& nodeName, bool active, const QModelIndex& parentIndex)
{
    // 通知模型：开始插入行
    beginInsertRows(parentIndex, 0, 0);

    // 通知模型：插入完成
    endInsertRows();

    // 返回新节点的索引
    return createIndex(0, 0, nullptr);
}

bool QuickTreeModel::removeNode(const QModelIndex& index)
{
    if (!index.isValid())
        return false;

    auto row = index.row();

    // 通知模型：开始删除行
    beginRemoveRows(parent(index), row, row);

    // 通知模型：删除完成
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
    // 通知模型：开始重置
    beginResetModel();

    // 通知模型：重置完成
    endResetModel();
}
} // namespace CSEditor
