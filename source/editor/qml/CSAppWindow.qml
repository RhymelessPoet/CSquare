import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import CSEditor.Theme 1.0
import CSEditor.View 1.0

ApplicationWindow {
    id: appWindow
    visible: true
    width: 1280
    height: 960
    title: "CSquare Editor"
    flags: Qt.FramelessWindowHint | Qt.Window
    color: "transparent"
    property int cachedVisibility: Window.Windowed

    onActiveChanged: {
        if (appWindow.active) {
            appWindow.visibility = cachedVisibility
        }
        else {
            cachedVisibility = appWindow.visibility
        }
    }

    // 背景圆角与阴影
    Rectangle {
        id: background
        anchors.fill: parent
        radius: 10
        color: CSTheme.background
        layer.enabled: true

        ColumnLayout {
            spacing: 2
            anchors.fill: parent
            
            CSTitleBar {
                id: titleBar
                window: appWindow
                radius: background.radius
                Layout.fillWidth: true
            }

            Rectangle {
                id: studio
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: CSTheme.surface

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 5
                    anchors.rightMargin: 5
                    spacing: 2

                    CSQuickRenderView {
                        id: renderView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                    Rectangle {
                        width: 380
                        Layout.fillHeight: true
                        color: CSTheme.surface
                    }
                }
            }

            Rectangle {
                id: statusBar
                Layout.fillWidth: true
                height: 30
                color: "transparent"
            }
        }
    }

}