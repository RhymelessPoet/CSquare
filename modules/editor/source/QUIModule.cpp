#include "QUIModule.h"
#include "UIApplication.h"
#include "engine/EngineController.h"
#include <QSurfaceFormat>
#include <QWindow>


namespace CS
{
struct QUIModuleImpl
{
    QUIModuleImpl(int argc, char* argv[]) : guiApp(argc, argv) {}

    std::shared_ptr<EngineController> engineController;
    UIApplication guiApp;
    bool everHadVisibleWindow{false};
};

QUIModule::QUIModule(int argc, char* argv[])
{
    qputenv("QSG_RHI_BACKEND", "opengl");
    // Force Qt's scene graph render to run on the main thread so it cannot
    // race with the engine's independent HGLRC via wglMakeCurrent.
    qputenv("QSG_RENDER_LOOP", "basic");
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setVersion(4, 5);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    m_impl = std::make_unique<QUIModuleImpl>(argc, argv);
}

void QUIModule::Initialize() {}

void QUIModule::Update()
{
    impl().guiApp.processEvents();

    // QGuiApplication::lastWindowClosed / aboutToQuit are only emitted
    // from within QGuiApplication::exec() (guarded by d->in_exec). Since
    // the engine drives its own loop via processEvents(), signal-based
    // hooks never fire — we must detect window closure by polling the
    // top-level window list. The latch (everHadVisibleWindow) prevents
    // exiting before the QML window first becomes visible (it is created
    // asynchronously via QTimer::singleShot in UIApplication).
    bool anyVisible = false;
    for (QWindow* w : QGuiApplication::topLevelWindows()) {
        if (w->isVisible()) {
            anyVisible = true;
            break;
        }
    }
    if (anyVisible) {
        impl().everHadVisibleWindow = true;
    } else if (impl().everHadVisibleWindow && impl().engineController) {
        impl().engineController->exit();
    }
}

void QUIModule::SetEngineController(std::shared_ptr<EngineController> controller)
{
    impl().engineController = std::move(controller);
}

QUIModuleImpl& QUIModule::impl()
{
    return *m_impl;
}

} // namespace CS
