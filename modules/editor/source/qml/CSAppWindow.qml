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
                    anchors.leftMargin: 0
                    anchors.rightMargin: 5
                    spacing: 3

                    // VS Code-style activity bar. Owns the hierarchy
                    // visibility flag and the samples-launcher menu.
                    CSSidebar {
                        id: sidebar
                        Layout.fillHeight: true
                    }

                    CSSceneHierarchyView {
                        id: sceneHierarchyView
                        // Width collapses to 0 when the sidebar toggle
                        // hides this panel, letting the render view
                        // reclaim the horizontal space.
                        visible: sidebar.hierarchyVisible
                        Layout.preferredWidth: visible ? 380 : 0
                        Layout.minimumWidth: visible ? 200 : 0
                        Layout.fillHeight: true
                    }

                    // Render view + overlay inspector stack. The inspector
                    // floats over the render view on the right edge; when
                    // collapsed it slides off, letting the render view
                    // reclaim the full area.
                    Item {
                        id: renderStack
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        CSQuickRenderView {
                            id: renderView
                            projectID: appWindow.projectID
                            anchors.fill: parent
                            focus: true

                            // Kick off asset loading only after the engine
                            // renderer/main View is ready.
                            onEngineReady: CSEditor.beginAssetLoading()
                        }

                        Connections {
                            target: CSEditor
                            // After the project's asset finishes loading
                            // and the scene has real geometry, fit the
                            // camera to the scene AABB.
                            function onSceneAssetLoaded() {
                                renderView.FitToScene(true)
                            }
                        }

                        CSInspectorView {
                            id: inspectorView
                            // Anchors handled inside the component (top/bottom/right).
                        }
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