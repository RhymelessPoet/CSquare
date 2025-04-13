#include "QUIModule.h"
#include "EngineController.h"
#include "UIApplication.h"

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
    m_impl = std::make_unique<QUIModuleImpl>(argc, argv);
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
