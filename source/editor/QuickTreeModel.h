#pragma once
#include <QAbstractItemModel>

namespace CSEditor
{

class QuickTreeModel : public QAbstractItemModel
{
    Q_OBJECT
    // 暴露根节点给 QML（可选）
    Q_PROPERTY(QModelIndex rootIndex READ rootIndex CONSTANT)

public:
    // 构造/析构
    explicit QuickTreeModel(QObject* parent = nullptr);
    ~QuickTreeModel() override;

    // ========== QAbstractItemModel 纯虚函数实现（核心） ==========
    // 返回指定索引对应的数据
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    // 设置指定索引的数据（支持 QML 修改）
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    // 返回索引的父索引
    QModelIndex parent(const QModelIndex& index) const override;
    // 返回指定父索引下的行数（子节点数量）
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    // 返回列数（固定为 1，适配 TreeView 单列展示）
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    // 创建索引（行/列/父索引）
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    // 返回支持的角色列表
    QHash<int, QByteArray> roleNames() const override;

    // ========== 扩展接口（节点管理） ==========
    // 获取根节点索引
    Q_INVOKABLE QModelIndex rootIndex() const;
    // 添加节点（parentIndex 为空则添加到根节点）
    Q_INVOKABLE QModelIndex addNode(const QString& nodeName,
                                    bool active = true,
                                    const QModelIndex& parentIndex = QModelIndex());
    // 删除指定节点
    Q_INVOKABLE bool removeNode(const QModelIndex& index);
    // 切换节点激活状态
    Q_INVOKABLE bool toggleNodeActive(const QModelIndex& index);
    // 切换节点展开状态
    Q_INVOKABLE bool toggleNodeExpanded(const QModelIndex& index);
    // 清空所有节点
    Q_INVOKABLE void clear();

private:
    QHash<int, QByteArray> m_roleNames; // 角色名称映射
};

} // namespace CSEditor
