#include "UIApplication.h"
#include "QuickRenderView.h"
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
