import QtQuick 2.15
import QtQuick.Window 2.15

import CSTheme 1.0

Window {
    id: window
    visible: true
    width: 1280
    height: 960
    title: "CSquare Editor"
    flags: Qt.FramelessWindowHint
    color: "transparent" 

    // 背景圆角与阴影
    Rectangle {
        id: background
        anchors.fill: parent
        radius: 10
        color: CSTheme.background
        layer.enabled: true

        // 标题栏拖拽区域
        Rectangle {
            id: titleBar
            width: parent.width
            color: "transparent"
            height: 40
            radius: parent.radius
            MouseArea {
                anchors.fill: parent
                onDoubleClicked: {
                    if (window.visibility === Window.Maximized) {
                        window.showNormal()
                    } else {
                        window.showMaximized()
                    }
                }
                onPressed: (mouse) => window.startSystemMove()
            }

            // 最小化/关闭按钮
            // Row {
            //     anchors.right: parent.right
            //     spacing: 10
            //     ImageButton { onClicked: window.showMinimized() }
            //     ImageButton { onClicked: toggleMaximized() }
            //     ImageButton { onClicked: window.close() }
            // }
        }
    }

}