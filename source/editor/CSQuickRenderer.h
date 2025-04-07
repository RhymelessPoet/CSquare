#include <QQuickFramebufferObject>

namespace CSEditor
{

class CSQuickRenderer : public QQuickFramebufferObject::Renderer
{
public:
    CSQuickRenderer(/* args */);
    ~CSQuickRenderer();

    virtual void render() override;
    virtual QOpenGLFramebufferObject* createFramebufferObject(const QSize& size) override;
    virtual void synchronize(QQuickFramebufferObject* fbo) override;
};

} // namespace CSEditor
