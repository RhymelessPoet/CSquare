import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CSEditor.Theme
import CSEditor.Model
import CSEditor.App


// Scene hierarchy panel: a standalone title row on top, a toolbar row
// (expand-all / collapse-all + search) on the second row, and the
// TreeView filling the rest of the area.
//
// The TreeView itself (internal `tree` id) is no longer the root so
// external layouts keep addressing this component as a single pane.
Item {
    id: root

    // Lower-cased search substring applied to node display text. Empty
    // means "no filter".
    property string filter: ""

    // Same minimalist hover-only button used by the Inspector. We avoid
    // QtQuick.Controls `Button` + `ToolTip` here too so hover popups
    // never trigger full scene-graph recomposites that could interact
    // with the sibling RHI render view.
    component ToolButton: Rectangle {
        id: toolBtn
        property string label: ""
        property string iconSource: ""
        signal triggered()
        implicitWidth: 28
        implicitHeight: 24
        color: hover.hovered ? CSTheme.secondaryBorder : CSTheme.surface
        border.color: CSTheme.primaryBorder
        border.width: 0.5
        radius: 3
        Image {
            anchors.centerIn: parent
            visible: toolBtn.iconSource.length > 0
            source: toolBtn.iconSource
            sourceSize.width: 16
            sourceSize.height: 16
            fillMode: Image.PreserveAspectFit
        }
        Label {
            anchors.centerIn: parent
            visible: toolBtn.iconSource.length === 0
            text: toolBtn.label
            color: CSTheme.textPrimary
        }
        HoverHandler { id: hover; cursorShape: Qt.PointingHandCursor }
        TapHandler { onTapped: toolBtn.triggered() }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 4

        // --- Title row (standalone) ---
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 4
            Layout.rightMargin: 4
            Layout.topMargin: 4
            spacing: 6

            Label {
                text: qsTr("Hierarchy")
                color: CSTheme.textPrimary
                font.bold: true
                font.pixelSize: 14
            }

            Item { Layout.fillWidth: true }
        }

        // --- Toolbar row: expand/collapse + search on the same line ---
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 4
            Layout.rightMargin: 4
            spacing: 6

            ToolButton {
                Layout.preferredWidth: 28
                Layout.preferredHeight: 24
                iconSource: "qrc:/CSQML/qml/icons/hierarchy/cs_hierarchy_expanded.png"
                onTriggered: tree.expandRecursively(-1, -1)
            }
            ToolButton {
                Layout.preferredWidth: 28
                Layout.preferredHeight: 24
                iconSource: "qrc:/CSQML/qml/icons/hierarchy/cs_hierarchy_collapsed.png"
                onTriggered: tree.collapseRecursively(-1)
            }

            TextField {
                id: searchBox
                Layout.fillWidth: true
                placeholderText: qsTr("Filter nodes\u2026")
                selectByMouse: true
                onTextChanged: {
                    root.filter = text.toLowerCase()
                    // Auto-expand the entire tree while a filter is
                    // active so deeply-nested matches become visible.
                    if (root.filter.length > 0) {
                        tree.expandRecursively(-1, -1)
                    }
                }
            }
        }

        // --- Tree ---
        TreeView {
            id: tree
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            boundsMovement: Flickable.StopAtBounds

            // Shared with the Inspector via QEditor::sceneSelection so both panels
            // observe the same current index without duplicating state.
            selectionModel: CSEditor.sceneSelection
            model: CSEditor.sceneTree

            // Background tap clears selection. This TapHandler is installed on the
            // TreeView itself; delegate TapHandlers accept LeftButton and run
            // first, consuming the event on hit rows. Only taps that land on the
            // empty area below / between delegates reach here.
            TapHandler {
                acceptedButtons: Qt.LeftButton
                onTapped: {
                    var selModel = tree.selectionModel
                    if (!selModel) return
                    selModel.clearCurrentIndex()
                    selModel.clearSelection()
                }
            }

            delegate: Rectangle {
                id: delegateItem

                // Highlight is driven exclusively by the delegate-provided `current`
                // flag, which reflects selectionModel.currentIndex. Do NOT mix with
                // `treeView.currentRow` — that is a separate keyboard-focus cursor
                // maintained by TreeView and would produce a second highlighted row.
                color: delegateItem.current ? CSTheme.secondary : CSTheme.background
                opacity: 0.3

                // Hide rows whose node name doesn't match the filter. When hidden
                // the row also collapses its height so it doesn't leave a gap.
                readonly property bool rowMatches: root.filter.length === 0
                    || (model.display !== undefined
                        && String(model.display).toLowerCase().indexOf(root.filter) !== -1)

                visible: rowMatches
                implicitWidth: tree.width
                implicitHeight: rowMatches ? label.implicitHeight * 1.5 : 0

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

                TapHandler {
                    acceptedButtons: Qt.LeftButton
                    // Make sure the delegate handler runs before the TreeView-level
                    // background tap handler so row taps are not treated as "blank
                    // area" clicks.
                    gesturePolicy: TapHandler.ReleaseWithinBounds
                    onTapped: {
                        var modelIdx = treeView.index(delegateItem.row, delegateItem.column)
                        var selModel = treeView.selectionModel
                        if (!selModel) return
                        var isSame = selModel.currentIndex === modelIdx
                        if (isSame) {
                            selModel.clearCurrentIndex()
                            selModel.clearSelection()
                        } else {
                            selModel.setCurrentIndex(modelIdx, ItemSelectionModel.ClearAndSelect)
                        }
                    }
                }

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
                    opacity: tree.movingVertically ? 0.3 : 0.0

                    Behavior on opacity {
                        OpacityAnimator {
                            duration: 500
                        }
                    }
                }
            }
        }
    }
}
