import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models

import CSEditor.Theme
import CSEditor.App

// VS Code-style activity bar. A fixed-width vertical strip that hosts
// toggle / action buttons. The top button drives Scene Hierarchy
// visibility, the bottom button pops up the render-sample launcher.
Rectangle {
    id: sidebar

    // Outward-facing state: bound by parent to control panel visibility.
    property bool hierarchyVisible: true

    // Width picked to match a 28px icon button plus 10px horizontal padding
    // on each side, matching VS Code's activity bar proportions.
    width: 48
    color: CSTheme.background
    // Subtle separator against the adjacent hierarchy / render panes.
    // Using the secondary border tone keeps the sidebar visually quiet
    // while still delineating its right edge.
    border.color: CSTheme.secondaryBorder
    border.width: 0.5

    // Top -> bottom stack with a flexible spacer in the middle so the
    // samples button anchors to the bottom regardless of sidebar height.
    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 8
        anchors.bottomMargin: 8
        spacing: 6

        // --- Top: Scene Hierarchy toggle ---
        SidebarButton {
            id: hierarchyButton
            Layout.alignment: Qt.AlignHCenter
            iconSource: "qrc:/CSQML/qml/icons/cs_sidebar_hierarchy.png"
            checkable: true
            checked: sidebar.hierarchyVisible
            tooltipText: "Toggle Scene Hierarchy"
            onClicked: sidebar.hierarchyVisible = !sidebar.hierarchyVisible
        }

        // Flexible spacer pushing the samples button to the bottom.
        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }

        // --- Bottom: Samples menu trigger ---
        SidebarButton {
            id: samplesButton
            Layout.alignment: Qt.AlignHCenter
            iconSource: "qrc:/CSQML/qml/icons/cs_sidebar_samples.png"
            tooltipText: "Run Render Sample"
            onClicked: samplesMenu.popupAtButton(samplesButton)
        }
    }

    // Minimal icon button used inside the sidebar. Kept local so the
    // sidebar owns its visual language (hover state, fixed size) without
    // leaking into the general CSIconButton style used elsewhere.
    component SidebarButton: Rectangle {
        id: btn
        property string iconSource: ""
        property string tooltipText: ""
        property bool checkable: false
        property bool checked: false
        property bool hovered: mouseArea.containsMouse
        signal clicked()

        implicitWidth: 32
        implicitHeight: 32
        radius: 5
        color: {
            if (btn.checkable && btn.checked) {
                return CSTheme.secondaryBorder
            }
            if (btn.hovered) {
                return CSTheme.secondaryBorder
            }
            return "transparent"
        }

        // Accent strip shown when this toggle is active. Matches the
        // familiar VS Code activity-bar affordance.
        Rectangle {
            visible: btn.checkable && btn.checked
            width: 2
            height: parent.height - 6
            anchors.left: parent.left
            anchors.leftMargin: -6
            anchors.verticalCenter: parent.verticalCenter
            color: CSTheme.primary
            radius: 1
        }

        Image {
            anchors.centerIn: parent
            source: btn.iconSource
            sourceSize.width: 22
            sourceSize.height: 22
            fillMode: Image.PreserveAspectFit
            opacity: btn.hovered || (btn.checkable && btn.checked) ? 1.0 : 0.85
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: btn.clicked()
        }

        ToolTip.text: btn.tooltipText
        ToolTip.visible: btn.hovered && btn.tooltipText.length > 0
        ToolTip.delay: 500
    }

    // Popup menu listing all render samples discovered at runtime. Its
    // entries are re-queried on every open so newly-deployed sample
    // executables appear without restarting the editor.
    ListModel {
        id: sampleListModel
    }

    Menu {
        id: samplesMenu

        function popupAtButton(anchorBtn) {
            // Rebuild the sample list model each time so the menu always
            // reflects what is currently on disk.
            sampleListModel.clear()
            const names = CSEditor.getSampleNames()
            for (var i = 0; i < names.length; ++i) {
                sampleListModel.append({ name: names[i] })
            }
            // Pop above the anchor button, flush against the sidebar's
            // right edge, matching the VS Code activity-bar behaviour.
            const pos = anchorBtn.mapToItem(sidebar, anchorBtn.width, 0)
            samplesMenu.popup(sidebar, Qt.point(pos.x + 2,
                pos.y - samplesMenu.height + anchorBtn.height))
        }

        // Disabled placeholder shown only when no samples are deployed.
        MenuItem {
            text: qsTr("(No samples deployed)")
            enabled: false
            visible: sampleListModel.count === 0
            height: visible ? implicitHeight : 0
        }

        // One MenuItem per discovered sample executable. Instantiator
        // keeps the menu entries in sync with sampleListModel without
        // requiring manual insert/remove juggling.
        Instantiator {
            model: sampleListModel
            delegate: MenuItem {
                required property string name
                text: name
                onTriggered: CSEditor.launchSample(name)
            }
            onObjectAdded: (index, object) => samplesMenu.insertItem(index, object)
            onObjectRemoved: (index, object) => samplesMenu.removeItem(object)
        }
    }
}
