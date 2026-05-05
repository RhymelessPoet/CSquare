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

    // Fixed row height keeps the delegate stable (using label.implicitHeight
    // causes brief flicker before the label is measured). 24px leaves room
    // for comfortably-sized expand/type icons (20px + breathing space).
    readonly property int rowHeight: 24

    // Panel background. Gives the panel a surface tone distinct from the
    // window and provides a single paint region for clipping.
    Rectangle {
        anchors.fill: parent
        color: CSTheme.background
    }

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
        anchors.margins: 6
        spacing: 6

        // --- Title row (standalone) ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Label {
                text: qsTr("Hierarchy")
                color: CSTheme.textPrimary
                font.bold: true
                font.pixelSize: 14
            }

            Item { Layout.fillWidth: true }
        }

        // Thin separator under the title to echo the Inspector header.
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: CSTheme.primaryBorder
            opacity: 0.6
        }

        // --- Toolbar row: expand/collapse + search on the same line ---
        RowLayout {
            Layout.fillWidth: true
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
                Layout.preferredHeight: 24
                placeholderText: qsTr("Filter nodes\u2026")
                placeholderTextColor: CSTheme.textSecondary
                color: CSTheme.textPrimary
                selectByMouse: true
                leftPadding: 8
                rightPadding: 8
                background: Rectangle {
                    color: CSTheme.surface
                    border.color: searchBox.activeFocus ? CSTheme.primary
                                                       : CSTheme.primaryBorder
                    border.width: searchBox.activeFocus ? 1 : 0.5
                    radius: 3
                }
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
            topMargin: 2
            bottomMargin: 4

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

                // Hide rows whose node name doesn't match the filter. When hidden
                // the row also collapses its height so it doesn't leave a gap.
                readonly property bool rowMatches: root.filter.length === 0
                    || (model.display !== undefined
                        && String(model.display).toLowerCase().indexOf(root.filter) !== -1)

                visible: rowMatches
                implicitWidth: tree.width
                implicitHeight: rowMatches ? root.rowHeight : 0

                readonly property real indentation: 16
                readonly property real padding: 6

                // Assigned to by TreeView:
                required property TreeView treeView
                required property bool isTreeNode
                required property bool expanded
                required property bool hasChildren
                required property int depth
                required property int row
                required property int column
                required property bool current

                // Base row is transparent; the layered rectangles below draw
                // the hover and selection tints without opacity bleeding into
                // children (icons, labels, indicators).
                color: "transparent"

                // Hover tint (painted under selection).
                Rectangle {
                    anchors.fill: parent
                    color: CSTheme.surface
                    visible: rowHover.hovered && !delegateItem.current
                    opacity: 0.55
                }

                // Selection fill. Subtle primary-tinted band rather than raw
                // secondary teal so it reads as a soft highlight, not a neon
                // stripe. Opacity is applied to this rect only; text and
                // icons stay fully opaque.
                Rectangle {
                    anchors.fill: parent
                    color: CSTheme.primary
                    visible: delegateItem.current
                    opacity: 0.22
                }

                // Left accent bar for the current row. Thin and flush-left.
                Rectangle {
                    visible: delegateItem.current
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 2
                    color: CSTheme.primary
                }

                // Indent guides: thin vertical rails at each depth step so
                // parent/child relationships are visible at a glance.
                Repeater {
                    model: delegateItem.depth
                    Rectangle {
                        width: 1
                        height: parent.height
                        x: delegateItem.padding + (index + 0.5) * delegateItem.indentation
                        color: CSTheme.secondaryBorder
                        opacity: 0.6
                    }
                }

                HoverHandler {
                    id: rowHover
                    cursorShape: Qt.PointingHandCursor
                }

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
                    anchors.leftMargin: delegateItem.depth * delegateItem.indentation + delegateItem.padding
                    anchors.rightMargin: 6
                    spacing: 4

                    CSIconButton {
                        id: indicator
                        Layout.preferredWidth: 20
                        Layout.preferredHeight: 20
                        // Match the button box so the chevron PNG is rendered
                        // at full 20px and not downscaled by PreserveAspectFit
                        // into the previous 16px slot (which made it look tiny).
                        iconSize: 20
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
                        Layout.preferredWidth: 18
                        Layout.preferredHeight: 18
                        source: "qrc:/CSQML/qml/icons/hierarchy/cs_hierarchy_%1.png".arg(model.object_type)
                        sourceSize.width: 18
                        sourceSize.height: 18
                        fillMode: Image.PreserveAspectFit
                        // Dim icons of inactive objects so visibility state is
                        // readable at a glance.
                        opacity: (model.active === undefined || model.active) ? 1.0 : 0.45
                    }

                    Label {
                        id: label
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        text: model.display
                        color: (model.active === undefined || model.active)
                            ? CSTheme.textPrimary
                            : CSTheme.textSecondary
                        font.bold: delegateItem.current
                    }

                    CSIconButton {
                        id: active
                        Layout.preferredWidth: 18
                        Layout.preferredHeight: 18
                        iconSize: 18
                        // Only reveal the eye on hover or on the selected row
                        // to keep the list visually quiet.
                        opacity: (rowHover.hovered || delegateItem.current || !model.active) ? 1.0 : 0.0
                        iconPath: model.active ? "qrc:/CSQML/qml/icons/hierarchy/cs_hierarchy_hide.png" : "qrc:/CSQML/qml/icons/hierarchy/cs_hierarchy_hide_checked.png"
                        onClicked: {
                            model.active = !model.active
                        }
                    }
                }
            }

            // Provide our own custom ScrollIndicator for the TreeView.
            // Stays faintly visible when the viewport is scrollable (tree
            // taller than the visible area) and brightens while actively
            // scrolling, so users can tell at a glance there is more below.
            ScrollIndicator.vertical: ScrollIndicator {
                active: true
                implicitWidth: 8

                contentItem: Rectangle {
                    implicitWidth: 6
                    implicitHeight: 6

                    readonly property bool scrollable: tree.contentHeight > tree.height

                    color: CSTheme.primary
                    radius: width / 2
                    opacity: tree.movingVertically ? 0.5
                                                   : (scrollable ? 0.18 : 0.0)

                    Behavior on opacity {
                        OpacityAnimator {
                            duration: 400
                        }
                    }
                }
            }
        }
    }
}
