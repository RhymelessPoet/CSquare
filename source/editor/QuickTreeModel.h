#pragma once
#include <QAbstractItemModel>

namespace CSEditor
{
class TreeModel;
class TreeNode;

class QuickTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit QuickTreeModel(TreeModel* model, QObject* parent = nullptr);
    explicit QuickTreeModel(QObject* parent = nullptr);
    ~QuickTreeModel() override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QModelIndex addNode(const QString& nodeName,
                                    bool active = true,
                                    const QModelIndex& parentIndex = QModelIndex());
    Q_INVOKABLE bool removeNode(const QModelIndex& index);

    Q_INVOKABLE bool toggleNodeActive(const QModelIndex& index);
    Q_INVOKABLE bool toggleNodeExpanded(const QModelIndex& index);

    Q_INVOKABLE void clear();

    void setModel(TreeModel* model);

private:
    void initialize();
    std::string getRoleName(int role) const;
    void constructNode(const QModelIndex& parent, const TreeNode* parentNode);

private:
    TreeModel* m_model{nullptr};
    QHash<int, QByteArray> m_roleNames;
};

} // namespace CSEditor
