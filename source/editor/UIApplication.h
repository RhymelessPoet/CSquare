#pragma once
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <memory>

namespace CSEditor
{
class QEditor;
} // namespace CSEditor

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
    std::unique_ptr<CSEditor::QEditor> m_editor;
};

} // namespace CS
