#include "ComputeResultItem.h"
#include "ComputeTestController.h"
#include "EdgePreservingFilterCase.h"
#include "LaplacianSmoothingCase.h"
#include <QElapsedTimer>
#include <QPainter>
#include <QQuickWindow>
#include <QSGSimpleTextureNode>

ComputeResultItem::ComputeResultItem(QQuickItem* parent) : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
    m_edgeCase = std::make_unique<EdgePreservingFilterCase>();
    m_laplacianCase = std::make_unique<LaplacianSmoothingCase>();
}

ComputeResultItem::~ComputeResultItem() = default;

QObject* ComputeResultItem::controllerObject() const
{
    return m_controller;
}
void ComputeResultItem::setControllerObject(QObject* controller)
{
    setController(qobject_cast<ComputeTestController*>(controller));
}

void ComputeResultItem::setController(ComputeTestController* controller)
{
    if (m_controller == controller) {
        return;
    }
    if (m_controller) {
        disconnect(m_controller, nullptr, this, nullptr);
    }
    m_controller = controller;
    if (m_controller) {
        connect(m_controller, &ComputeTestController::runRequested, this, [this] { update(); });
    }
    emit controllerChanged();
}

QSGNode* ComputeResultItem::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{
    auto* node = static_cast<QSGSimpleTextureNode*>(oldNode);
    if (!node) {
        node = new QSGSimpleTextureNode;
        node->setOwnsTexture(true);
        QImage placeholder(1, 1, QImage::Format_RGBA8888);
        placeholder.fill(Qt::transparent);
        node->setTexture(window()->createTextureFromImage(placeholder));
    }
    int caseIndex = 0;
    if (m_controller && m_controller->takeRunRequest(caseIndex)) {
        QElapsedTimer timer;
        timer.start();
        QString error;
        QImage image = caseIndex == 0 ? m_edgeCase->run(error) : m_laplacianCase->run(error);
        if (image.isNull()) {
            image = QImage(960, 600, QImage::Format_RGBA8888);
            image.fill(QColor("#522b2b"));
        }
        auto* texture = window()->createTextureFromImage(image);
        node->setTexture(texture);
        node->setRect(boundingRect());
        m_controller->completeRun(image, timer.nsecsElapsed() / 1.0e6, 0.0, error);
    } else {
        node->setRect(boundingRect());
    }
    update(); // Request another update to keep the item responsive
    return node;
}
