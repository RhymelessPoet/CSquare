#pragma once
#include <QQuickItem>
#include <memory>

class ComputeTestController;
class EdgePreservingFilterCase;
class LaplacianSmoothingCase;

class ComputeResultItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QObject* controller READ controllerObject WRITE setControllerObject NOTIFY controllerChanged)
public:
    explicit ComputeResultItem(QQuickItem* parent = nullptr);
    ~ComputeResultItem() override;
    ComputeTestController* controller() const { return m_controller; }
    void setController(ComputeTestController* controller);
    QObject* controllerObject() const;
    void setControllerObject(QObject* controller);
signals:
    void controllerChanged();

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;

private:
    ComputeTestController* m_controller{nullptr};
    std::unique_ptr<EdgePreservingFilterCase> m_edgeCase;
    std::unique_ptr<LaplacianSmoothingCase> m_laplacianCase;
};
