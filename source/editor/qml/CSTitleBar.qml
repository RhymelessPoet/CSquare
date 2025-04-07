import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import CSEditor.Theme 1.0

// 标题栏拖拽区域
Rectangle {
    id: titleBar

    property Window window: parent

    width: parent.width
    height: 32
    color: "transparent"

    RowLayout {
        // property alias window: titleBar.window
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        spacing: 5

        Image {
            source: "qrc:/CSQML/qml/icons/cslogo.png"
            Layout.preferredWidth: 28
            Layout.preferredHeight: 28
        }

        Text { 
            text: "CSEditor";
            color: CSTheme.textPrimary
            font.pixelSize: 14
        }
        // spacing
        Item {
            Layout.fillWidth: true
            height: parent.height
            MouseArea {
                anchors.fill: parent
                onDoubleClicked: {
                    if (window.visibility === Window.Maximized) {
                        window.showNormal()
                    } else {
                        window.showMaximized()
                    }
                }
                onPressed: (mouse) => window.startSystemMove()
            }
        }
        CSIconButton {
            id: minimizeButton
            iconPath: "qrc:/CSQML/qml/icons/cs_title_bar_minimize.png"
            Layout.preferredWidth: 32
            Layout.preferredHeight: 28

            onClicked: window.showMinimized()
        }
        CSIconButton {
            id: maximizeButton
            checkable: true
            iconPath: "qrc:/CSQML/qml/icons/cs_title_bar_maximize.png"
            Layout.preferredWidth: 32
            Layout.preferredHeight: 28

            onClicked: window.visibility === Window.Maximized ? 
                    window.showNormal() : window.showMaximized()

            onCheckedChanged: {
                console.log(maximizeButton.checked)
                if (maximizeButton.checked) {
                    iconPath = "qrc:/CSQML/qml/icons/cs_title_bar_maximized.png"
                }
                else {
                    iconPath = "qrc:/CSQML/qml/icons/cs_title_bar_maximize.png"
                }
            }
        }
        CSIconButton {
            id: closeButton
            iconPath: "qrc:/CSQML/qml/icons/cs_title_bar_close.png"
            Layout.preferredWidth: 32
            Layout.preferredHeight: 28
            stateMaskColor: "#FF0000"
            onClicked: window.close()
        }
    }
}
