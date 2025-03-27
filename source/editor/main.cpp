#include <iostream>
#include <QQmlApplicationEngine>
#include <QGuiApplication>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    
    QQmlApplicationEngine engine;
    // 添加模块搜索路径
    engine.addImportPath("qrc:/");

    qmlRegisterSingletonType(QStringLiteral("qrc:/CSQML/qml/ThemePalette.qml"), "CSTheme", 1, 0, "CSTheme");
    engine.load(QUrl("qrc:/CSQML/qml/Main.qml"));
    
    return app.exec();
}
