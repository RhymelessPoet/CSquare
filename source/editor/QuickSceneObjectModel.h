#pragma once
#include <QAbstractListModel>
#include <QHash>
#include <QString>
#include <QVariant>

#include <memory>
#include <vector>

namespace CSEditor
{
class SceneObjectModel;
class QuickComponentModel;

// Qt adapter that projects a pure-C++ SceneObjectModel into a QML-friendly
// QAbstractListModel. Rows are the SceneObject's components; scalar header
// fields (name / active / hasSelection) are exposed as Q_PROPERTYs.
class QuickSceneObjectModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY changed)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY changed)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY changed)
public:
    enum Roles
    {
        ComponentRole = Qt::UserRole + 1,
    };

    explicit QuickSceneObjectModel(SceneObjectModel* model, QObject* parent = nullptr);
    ~QuickSceneObjectModel() override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString name() const;
    void setName(const QString& value);

    bool active() const;
    void setActive(bool value);

    bool hasSelection() const;

signals:
    void changed();

private:
    void onDomainChanged();
    void rebuild();

    SceneObjectModel* m_model{nullptr};
    std::vector<std::unique_ptr<QuickComponentModel>> m_quickComponents;
};

} // namespace CSEditor
