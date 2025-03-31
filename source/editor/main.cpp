#include <iostream>
#include <QQmlApplicationEngine>
#include <QGuiApplication>
#include <QQuickStyle>
#include <QIcon>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    
    app.setWindowIcon(QIcon(":/CSQML/qml/icons/cslogo.png"));
    QQuickStyle::setStyle("Fusion");

    QQmlApplicationEngine engine;
    // 添加模块搜索路径
    engine.addImportPath("qrc:/");

    qmlRegisterSingletonType(QStringLiteral("qrc:/CSQML/qml/CSThemePalette.qml"), "CSTheme", 1, 0, "CSTheme");
    engine.load(QUrl("qrc:/CSQML/qml/CSAppWindow.qml"));
    
    return app.exec();
}
