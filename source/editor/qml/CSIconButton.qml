import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import CSEditor.Theme

Button {
    id: button
    hoverEnabled: true

    Layout.preferredWidth: 24
    Layout.preferredHeight: 24

    // Let the icon fill the full button box. The default QtQuickControls
    // Button style reserves ~6px padding on each side, which was silently
    // shrinking our icons (e.g. a 20x20 PNG was scaled down to 8x8 inside
    // a 20x20 button).
    padding: 0

    property alias stateMaskColor: background.color
    property real stateMaskAlpha: {
        if (!enabled) {
            return 0.0
        } else if (down) {
            return 0.2
        } else if (hovered) {
            return 0.1
        } else {
            return 0.0
        }
    }

    property alias borderRadius: background.radius

    property alias iconPath: icon.source

    // Rasterization size of the icon PNG. Set this to match the final
    // display size to avoid blur / unnecessary downscale.
    property int iconSize: 20

    contentItem: Image {
        id: icon
        source: ""
        sourceSize.width: button.iconSize
        sourceSize.height: button.iconSize
        fillMode: Image.PreserveAspectFit
    }

    background: Rectangle {
        id: background
        anchors.fill: parent
        color: "#FFFFFF"
        opacity: button.stateMaskAlpha
        radius: 5
    }
}