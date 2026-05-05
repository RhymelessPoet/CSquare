#pragma once
#include <QAbstractListModel>
#include <QHash>
#include <QString>
#include <QVariant>

namespace CSEditor
{
class ComponentModel;

// Qt adapter that projects a pure-C++ ComponentModel into a QML-friendly
// QAbstractListModel. One row per PropertyItem.
class QuickComponentModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString componentName READ componentName CONSTANT)
public:
    enum Roles
    {
        UINameRole = Qt::UserRole + 1,
        TypeTagRole, // one of PropertyType enum values
        ValueRole,
        WritableRole,
    };

    explicit QuickComponentModel(ComponentModel* model, QObject* parent = nullptr);
    ~QuickComponentModel() override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    QString componentName() const;

    // QML-friendly edit entry point. Returns true if the value was applied.
    Q_INVOKABLE bool setValue(int row, const QVariant& value);

    // Tells QML views to re-query data (e.g. after a domain-side refresh).
    void notifyAllChanged();

private:
    ComponentModel* m_model{nullptr};
};

} // namespace CSEditor
