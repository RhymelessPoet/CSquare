#pragma once
#include <QGuiApplication>
#include <QQmlApplicationEngine>

namespace CS
{
class UIApplication : public QGuiApplication
{
public:
    UIApplication(int argc, char* argv[]);
    ~UIApplication();

    bool notify(QObject*, QEvent*) override;

private:
    QQmlApplicationEngine m_qmlEngine;
};

} // namespace CS
