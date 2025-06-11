#include "QUIModule.h"
#include "EngineController.h"
#include "UIApplication.h"
#include <QSurfaceFormat>

namespace CS
{
struct QUIModuleImpl
{
    QUIModuleImpl(int argc, char* argv[]) : guiApp(argc, argv) {}

    std::shared_ptr<EngineController> engineController;
    UIApplication guiApp;
};

QUIModule::QUIModule(int argc, char* argv[])
{
    qputenv("QSG_RHI_BACKEND", "opengl");
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setVersion(4, 5);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    m_impl = std::make_unique<QUIModuleImpl>(argc, argv);

    // HGLRC qtHGLRC = ...;                            // 从 Qt 上下文获取
    // wglShareLists(qtHGLRC, wglGetCurrentContext()); // 关键：共享资源列表
}

void QUIModule::Update()
{
    impl().guiApp.processEvents();
}

void QUIModule::SetEngineController(std::shared_ptr<EngineController> controller)
{
    impl().engineController = std::move(controller);
    QObject::connect(&impl().guiApp, &QGuiApplication::aboutToQuit, [=]() { impl().engineController->exit(); });
}

QUIModuleImpl& QUIModule::impl()
{
    return *m_impl;
}

} // namespace CS
