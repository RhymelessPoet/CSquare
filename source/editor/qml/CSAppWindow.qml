import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import CSEditor.Theme
import CSEditor.View
import CSEditor.App

ApplicationWindow {
    id: appWindow
    visible: true
    width: 1280
    height: 960
    title: "CSquare Editor"
    flags: Qt.FramelessWindowHint | Qt.Window
    color: "transparent"
    property int cachedVisibility: Window.Windowed
    property string projectID: ""

    onActiveChanged: {
        if (appWindow.active) {
            appWindow.visibility = cachedVisibility
        }
        else {
            cachedVisibility = appWindow.visibility
        }
    }

    Component.onCompleted: {
        CSEditor.loadProject();
        appWindow.projectID = CSEditor.getProjectID();
    }

    // 背景圆角与阴影
    Rectangle {
        id: background
        anchors.fill: parent
        radius: 10
        color: CSTheme.background
        layer.enabled: true

        border.pixelAligned: false
        border.color: CSTheme.primaryBorder
        border.width: 0.5

        states: [
            State {
                when: appWindow.visibility === Window.Maximized
                PropertyChanges { target: background; radius: 0; border.width: 0.0 }
            }
        ]

        ColumnLayout {
            spacing: 2
            anchors.fill: parent
            anchors.leftMargin: background.border.width
            anchors.rightMargin: background.border.width
            
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
                    anchors.leftMargin: 2
                    anchors.rightMargin: 5
                    spacing: 3

                    CSSceneHierarchyView {
                        id: sceneHierarchyView
                        Layout.preferredWidth: 380
                        Layout.minimumWidth: 200
                        Layout.fillHeight: true
                    }

                    CSQuickRenderView {
                        id: renderView
                        projectID: appWindow.projectID
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                    CSInspectorView {
                        id: inspectorView
                        Layout.preferredWidth: 360
                        Layout.minimumWidth: 240
                        Layout.fillHeight: true
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