#pragma once
#include <QObject>
#include <QString>
#include <memory>

namespace CSEditor
{
class ProjectModel;

class QEditor : public QObject
{
    Q_OBJECT
public:
    QEditor(/* args */);
    ~QEditor();

    Q_INVOKABLE void loadProject();
    Q_INVOKABLE const QString getProjectID() const;

private:
    QString m_projectID;
};

} // namespace CSEditor
