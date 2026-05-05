#include "UIApplication.h"
#include "QEditor.h"
#include "QuickComponentModel.h"
#include "QuickRenderView.h"
#include "QuickSceneObjectModel.h"
#include "QuickTreeModel.h"
#include <QIcon>
#include <QQuickStyle>

namespace CS
{
UIApplication::UIApplication(int argc, char* argv[]) : QGuiApplication(argc, argv)
{
    setWindowIcon(QIcon(":/CSQML/qml/icons/cslogo.png"));
    QQuickStyle::setStyle("Fusion");

    m_qmlEngine.addImportPath("qrc:/");
    qmlRegisterType<CSEditor::QuickRenderView>("CSEditor.View", 1, 0, "CSQuickRenderView");
    qmlRegisterType<CSEditor::QuickTreeModel>("CSEditor.Model", 1, 0, "CSQuickTreeModel");
    qmlRegisterUncreatableType<CSEditor::QuickSceneObjectModel>("CSEditor.Model", 1, 0, "CSQuickSceneObjectModel",
                                                                "created in C++");
    qmlRegisterUncreatableType<CSEditor::QuickComponentModel>("CSEditor.Model", 1, 0, "CSQuickComponentModel",
                                                              "created in C++");

    m_editor = std::make_unique<CSEditor::QEditor>();
    qmlRegisterSingletonType<CSEditor::QEditor>(
        "CSEditor.App", 1, 0, "CSEditor", [=](QQmlEngine* engine, QJSEngine* scriptEngine) { return m_editor.get(); });

    qmlRegisterSingletonType(QStringLiteral("qrc:/CSQML/qml/CSThemePalette.qml"), "CSEditor.Theme", 1, 0, "CSTheme");

    m_qmlEngine.load(QUrl("qrc:/CSQML/qml/CSAppWindow.qml"));
}

UIApplication::~UIApplication() {}

bool UIApplication::notify(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::Close) {
        exit();
    }
    return QGuiApplication::notify(object, event);
}

} // namespace CS
