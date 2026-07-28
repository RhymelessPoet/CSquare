#include "ComputeResultItem.h"
#include "ComputeTestController.h"
#include "base/Logger.h"
#include <QGuiApplication>
#include <QProcessEnvironment>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSurfaceFormat>
#include <QUrl>

int main(int argc, char* argv[])
{
    CS::Logger::Initialize("test_compute_pipeline.log", CS::LogSinkValues::Both(),
                           CS::LogLevels::Error() | CS::LogLevels::Debug() | CS::LogLevels::Info(), false, true,
                           10 * 1024 * 1024);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("QSG_ALWAYS_UPDATE", "1");

    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setVersion(4, 5);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    QSurfaceFormat::setDefaultFormat(format);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    QGuiApplication app(argc, argv);
    QGuiApplication::setOrganizationName(QStringLiteral("CSquare"));
    QGuiApplication::setApplicationName(QStringLiteral("ComputePipelineTest"));

    qmlRegisterType<ComputeResultItem>("CSComputePipelineTest", 1, 0, "ComputeResultItem");
    ComputeTestController controller;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("testController"), &controller);
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app, [] { QCoreApplication::exit(EXIT_FAILURE); },
        Qt::QueuedConnection);
    engine.load(QUrl(QStringLiteral("qrc:/CSComputePipelineTest/Main.qml")));
    QMetaObject::invokeMethod(&controller, &ComputeTestController::requestRun, Qt::QueuedConnection);

    auto result = app.exec();

    CS::Logger::Shutdown();

    return result;
}
