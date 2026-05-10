import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CSEditor.Theme
import CSEditor.App

// Inspector panel for the currently-selected SceneObject, rendered as an
// overlay drawer on the right edge of its parent. When collapsed, the
// panel slides off-screen so only a thin edge-tab remains clickable; the
// area underneath (the render view) is reclaimed.
//
// The QML layer only sees `CSEditor.sceneObjectModel` (a QAbstractListModel
// adapter), never the underlying domain model.
Item {
    id: inspector

    // Panel geometry. panelWidth = drawer body; tabWidth = always-visible
    // toggle strip on the left side of the drawer (hit area).
    // tabVisualWidth = the visible chip width; kept smaller than tabWidth so
    // the tab looks slim but remains easy to click.
    property int panelWidth: 360
    property int tabWidth: 14
    property int tabVisualWidth: 6
    property bool expanded: false

    readonly property var som: CSEditor.sceneObjectModel

    // Toolbar button. We deliberately avoid QtQuick.Controls `Button` +
    // `ToolTip` in the drawer: the Qt Quick Controls `Popup` used by
    // `ToolTip` opens in the ApplicationWindow overlay, which forces a
    // full scene-graph recomposite. Because our sibling
    // CSQuickRenderView is a QQuickRhiItem whose frame texture is
    // composited into the window, the Popup's fade animation resamples
    // that texture with transient alpha, producing a ghost / washed-out
    // render frame while the pointer hovers the button. A plain
    // Rectangle + HoverHandler never opens a Popup, so the RHI texture
    // is untouched.
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

    // The root covers the full overlay area; mouse events only hit the
    // drawer and tab below, so the render view beneath remains interactive
    // wherever the drawer is not.
    //
    // `z` forces this Item strictly above the sibling QQuickRhiItem render
    // view. Without it, Qt Quick may deliver hover events to the RHI item
    // (which has setAcceptHoverEvents(true)) while a Popup/ToolTip inside
    // the drawer is opening, producing a ghost/washed-out render frame.
    //
    // `clip: true` keeps hover/tooltip repaints inside our bounds so the
    // scene-graph damage region never overlaps the RHI texture rectangle.
    z: 1
    clip: true
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    anchors.right: parent.right
    width: panelWidth + tabWidth

    // --- Edge tab: always-visible toggle anchored to the drawer's left edge.
    // The outer Item is the hit area (tabWidth wide); the inner Rectangle is
    // the visible chip (tabVisualWidth wide, right-aligned so it hugs the
    // drawer). This keeps the button easy to click while making the visible
    // strip slimmer.
    Item {
        id: edgeTab
        width: inspector.tabWidth
        height: 64
        anchors.verticalCenter: parent.verticalCenter
        x: drawer.x - width

        Rectangle {
            id: edgeTabVisual
            width: inspector.tabVisualWidth
            height: parent.height
            anchors.right: parent.right
            color: CSTheme.surface
            border.color: CSTheme.primaryBorder
            border.width: 0.5
            radius: 3

            Label {
                anchors.centerIn: parent
                // Mathematical angle brackets (U+27E8 / U+27E9): single
                // thin strokes that are narrower and taller than ASCII
                // '<' / '>', fitting the slim edge-tab chip.
                text: inspector.expanded ? "\u27E9" : "\u27E8"
                color: CSTheme.textSecondary
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: inspector.expanded = !inspector.expanded
        }
    }

    // --- Sliding drawer ---
    Rectangle {
        id: drawer
        width: inspector.panelWidth
        height: parent.height
        anchors.top: parent.top
        // Expanded: occupy the panel slot on the right. Collapsed: slide
        // fully off to the right so only the edgeTab (glued to drawer.x)
        // remains visible.
        x: inspector.expanded ? (inspector.width - width) : inspector.width
        color: CSTheme.background
        border.color: CSTheme.primaryBorder
        border.width: 0.5
        // Clip internal child repaints (button hover animations, ToolTip
        // popup's anchor rect, etc.) so they don't leak outside the drawer
        // bounds onto the underlying RHI render view.
        clip: true

        Behavior on x {
            NumberAnimation { duration: 180; easing.type: Easing.InOutQuad }
        }

        // Filter string (lower-cased) fed into each CSComponentView.
        property string filter: ""

        signal expandAllRequested(bool expand)

        // Transparent hover/mouse absorber AND blank-click focus sink.
        //
        // Hover side: the QQuickRhiItem underneath declares
        // setAcceptHoverEvents(true); without absorbing hover here, hover
        // crossings at the drawer edge cascade to the RHI item and race
        // against ToolTip popups, producing the ghost-frame artifact.
        //
        // Click side: this MouseArea is the *parent* of all drawer UI
        // (focusSink + ColumnLayout below are its children). Qt Quick mouse
        // events bubble up the parent chain only, so a sibling MouseArea
        // would not receive clicks that fall through inner Labels /
        // Rectangles. As a parent, we receive any press that descendant
        // controls (TextFields, Buttons, drag handles) did not accept, and
        // pull active focus to `focusSink`. That commits any in-flight
        // TextField edit via its onEditingFinished signal.
        MouseArea {
            id: drawerClickSink
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            propagateComposedEvents: false
            onPressed: focusSink.forceActiveFocus()

            // Invisible focus sink. Any time we want to defocus an active
            // TextField (and trigger its onEditingFinished commit), we
            // park focus here.
            Item {
                id: focusSink
                width: 1
                height: 1
                activeFocusOnTab: false
            }

            // Filter string (lower-cased) fed into each CSComponentView.
            // Hosted on the MouseArea so existing `drawer.filter`
            // references keep working.
            // (Moved out of `drawer` so the filter property and the
            // expandAllRequested signal stay in scope for the children.)

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 6
                spacing: 6

            // --- Title row (standalone) ---
            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Label {
                    text: qsTr("Inspector")
                    color: CSTheme.textPrimary
                    font.bold: true
                    font.pixelSize: 14
                }

                Item { Layout.fillWidth: true }

                ToolButton {
                    Layout.preferredWidth: 28
                    Layout.preferredHeight: 24
                    label: "\u2715"
                    onTriggered: inspector.expanded = false
                }
            }

            // --- Toolbar row: expand/collapse + search on the same line ---
            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                // Single toggle: expands all when collapsed, collapses
                // all when expanded. Icon reflects the current aggregate
                // state so clicking flips it.
                ToolButton {
                    id: expandToggle
                    property bool allExpanded: false
                    Layout.preferredWidth: 28
                    Layout.preferredHeight: 24
                    iconSource: allExpanded
                        ? "qrc:/CSQML/qml/icons/hierarchy/cs_hierarchy_expanded.png"
                        : "qrc:/CSQML/qml/icons/hierarchy/cs_hierarchy_collapsed.png"
                    onTriggered: {
                        allExpanded = !allExpanded
                        drawer.expandAllRequested(allExpanded)
                    }
                }

                TextField {
                    id: searchBox
                    Layout.fillWidth: true
                    placeholderText: qsTr("Filter properties\u2026")
                    selectByMouse: true
                    onTextChanged: drawer.filter = text.toLowerCase()
                }
            }

            // --- Empty state ---
            Label {
                visible: !inspector.som || !inspector.som.hasSelection
                text: qsTr("No SceneObject selected")
                color: CSTheme.textSecondary
                Layout.fillWidth: true
                Layout.margins: 10
                horizontalAlignment: Text.AlignHCenter
            }

            // --- Header: name + active ---
            Rectangle {
                visible: inspector.som && inspector.som.hasSelection
                Layout.fillWidth: true
                implicitHeight: header.implicitHeight + 10
                color: CSTheme.surface
                radius: 4

                RowLayout {
                    id: header
                    anchors.fill: parent
                    anchors.margins: 5
                    spacing: 8

                    // Eye / hide toggle. Mirrors the hierarchy row's active
                    // indicator: open eye when the SceneObject is active,
                    // crossed-out eye when hidden. Kept as a plain Image +
                    // MouseArea (no QtQuick.Controls Button) so hover /
                    // click does not trigger any Popup that could ghost the
                    // sibling CSQuickRenderView behind the drawer.
                    Image {
                        id: activeToggle
                        Layout.preferredWidth: 14
                        Layout.preferredHeight: 14
                        sourceSize.width: 14
                        sourceSize.height: 14
                        fillMode: Image.PreserveAspectFit
                        source: (inspector.som && inspector.som.active)
                            ? "qrc:/CSQML/qml/icons/hierarchy/cs_hierarchy_hide.png"
                            : "qrc:/CSQML/qml/icons/hierarchy/cs_hierarchy_hide_checked.png"
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (inspector.som) {
                                    inspector.som.active = !inspector.som.active
                                }
                            }
                        }
                    }

                    TextField {
                        Layout.fillWidth: true
                        text: inspector.som ? inspector.som.name : ""
                        onEditingFinished: if (inspector.som && text !== inspector.som.name) inspector.som.name = text
                    }
                }
            }

            // --- Components list ---
            ScrollView {
                id: scroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                // Wrap compColumn in a MouseArea-as-parent so blank-area
                // clicks inside the scroll viewport (gaps between
                // components, area below the last component, blank
                // CSComponentView background) bubble up here and pull
                // active focus to `focusSink`. Qt Quick mouse events bubble
                // up the parent chain only, so a sibling MouseArea would
                // miss these. ScrollView's internal Flickable also blocks
                // events from reaching the drawer-level sink.
                MouseArea {
                    id: scrollClickSink
                    width: scroll.availableWidth
                    // At least fill the viewport so blank space below the
                    // last component still routes presses here. Grow with
                    // content when components overflow so the Flickable
                    // can scroll normally.
                    implicitHeight: Math.max(compColumn.implicitHeight, scroll.availableHeight)
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    onPressed: focusSink.forceActiveFocus()

                    ColumnLayout {
                        id: compColumn
                        // Top-anchor + width-only: keep components stacked
                        // at the top with their implicit heights. Letting
                        // anchors.fill: parent stretch the layout to the
                        // (now viewport-sized) MouseArea would distribute
                        // empty space between components.
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        spacing: 4

                        Repeater {
                            id: compRepeater
                            model: (inspector.som && inspector.som.hasSelection) ? inspector.som : null
                            delegate: CSComponentView {
                                Layout.fillWidth: true
                                componentModel: model.component
                                filter: drawer.filter
                                // Hide components whose name does not match the
                                // filter. (Per-property filtering lives inside
                                // CSComponentView via rowVisible; matching at the
                                // component level just controls container visibility.)
                                visible: componentNameMatches

                                Connections {
                                    target: drawer
                                    function onExpandAllRequested(state) { applyExpand(state) }
                                }
                            }
                        }
                    }
                }
            }
        }
        }
    }
}
