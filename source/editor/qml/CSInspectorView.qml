import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CSEditor.Theme
import CSEditor.App

// Inspector panel for the currently-selected SceneObject.
// The QML layer only sees `CSEditor.sceneObjectModel` (a QAbstractListModel
// adapter), never the underlying domain model.
ScrollView {
    id: inspector
    clip: true
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

    readonly property var som: CSEditor.sceneObjectModel

    ColumnLayout {
        width: inspector.availableWidth
        spacing: 4

        // --- Empty state ---
        Label {
            visible: !som || !som.hasSelection
            text: qsTr("No SceneObject selected")
            color: CSTheme.textSecondary
            Layout.fillWidth: true
            Layout.margins: 10
            horizontalAlignment: Text.AlignHCenter
        }

        // --- Header: name + active ---
        Rectangle {
            visible: som && som.hasSelection
            Layout.fillWidth: true
            Layout.margins: 6
            implicitHeight: header.implicitHeight + 10
            color: CSTheme.surface
            radius: 4

            RowLayout {
                id: header
                anchors.fill: parent
                anchors.margins: 5
                spacing: 8

                CheckBox {
                    checked: som ? som.active : false
                    onToggled: if (som) som.active = checked
                }

                TextField {
                    Layout.fillWidth: true
                    text: som ? som.name : ""
                    onEditingFinished: if (som && text !== som.name) som.name = text
                }
            }
        }

        // --- Components list ---
        Repeater {
            model: (som && som.hasSelection) ? som : null
            delegate: CSComponentView {
                Layout.fillWidth: true
                componentModel: model.component
            }
        }
    }
}
