import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CSEditor.Theme
import CSEditor.Model
import CSEditor.App


TreeView {
    id: treeView
    clip: true
    boundsBehavior: Flickable.StopAtBounds
    boundsMovement: Flickable.StopAtBounds

    selectionModel: ItemSelectionModel {}
    model: CSEditor.sceneTree

    delegate: Rectangle {
        id: delegateItem

        color: row === treeView.currentRow ? CSTheme.secondary : CSTheme.background
        opacity: 0.3

        implicitWidth: treeView.width
        implicitHeight: label.implicitHeight * 1.5

        readonly property real indentation: 20
        readonly property real padding: 5

        // Assigned to by TreeView:
        required property TreeView treeView
        required property bool isTreeNode
        required property bool expanded
        required property bool hasChildren
        required property int depth
        required property int row
        required property int column
        required property bool current

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: depth * indentation + padding
            anchors.rightMargin: 5
            spacing: 2
        
            CSIconButton {
                id: indicator
                opacity: hasChildren ? 1.0 : 0.0
                enabled: hasChildren
                iconPath: model.expand ? "qrc:/CSQML/qml/icons/hierarchy/cs_hierarchy_expanded.png" : "qrc:/CSQML/qml/icons/hierarchy/cs_hierarchy_collapsed.png"
                onClicked: {
                    if (model.expand) {
                        model.expand = false
                    } else {
                        model.expand = true
                    }
                    treeView.toggleExpanded(delegateItem.row)
                }
            }

            Image {
                source: "qrc:/CSQML/qml/icons/hierarchy/cs_hierarchy_%1.png".arg(model.object_type)
                sourceSize.width: 16
                sourceSize.height: 16
                fillMode: Image.PreserveAspectFit
            }

            Label {
                id: label
                width: parent.width - padding - x
                clip: true
                text: model.display
                color: CSTheme.textPrimary
            }
            Item {
                Layout.fillWidth: true
                height: parent.height
            }
            CSIconButton {
                id: active
                iconPath: model.active ? "qrc:/CSQML/qml/icons/hierarchy/cs_hierarchy_hide.png" : "qrc:/CSQML/qml/icons/hierarchy/cs_hierarchy_hide_checked.png"
                onClicked: {
                    model.active = !model.active
                }
            }
        }
    }

    // Provide our own custom ScrollIndicator for the TreeView.
    ScrollIndicator.vertical: ScrollIndicator {
        active: true
        implicitWidth: 8

        contentItem: Rectangle {
            implicitWidth: 6
            implicitHeight: 6

            color: CSTheme.primary
            radius: width / 2
            opacity: treeView.movingVertically ? 0.3 : 0.0

            Behavior on opacity {
                OpacityAnimator {
                    duration: 500
                }
            }
        }
    }

}