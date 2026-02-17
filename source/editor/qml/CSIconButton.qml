import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import CSEditor.Theme

Button {
    id: button
    hoverEnabled: true

    Layout.preferredWidth: 24
    Layout.preferredHeight: 24

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

    contentItem: Image {
        id: icon
        source: ""
        sourceSize.width: 20
        sourceSize.height: 20
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