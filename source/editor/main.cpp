#include "CSQuickRenderView.h"
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

int main(int argc, char* argv[])
{
    qputenv("QSG_RHI_BACKEND", "opengl");
    QGuiApplication app(argc, argv);

    app.setWindowIcon(QIcon(":/CSQML/qml/icons/cslogo.png"));
    QQuickStyle::setStyle("Fusion");

    QQmlApplicationEngine engine;
    // 添加模块搜索路径
    engine.addImportPath("qrc:/");
    qmlRegisterType<CSEditor::CSQuickRenderView>("CSEditor.View", 1, 0, "CSQuickRenderView");
    qmlRegisterSingletonType(QStringLiteral("qrc:/CSQML/qml/CSThemePalette.qml"), "CSEditor.Theme", 1, 0, "CSTheme");

    engine.load(QUrl("qrc:/CSQML/qml/CSAppWindow.qml"));

    return app.exec();
}
