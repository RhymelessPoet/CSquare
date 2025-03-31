import QtQuick 2.15

pragma Singleton
QtObject {
    readonly property color background: "#121212"      // 主界面背景
    readonly property color surface: "#1E1E1E"         // 卡片、浮层背景
    readonly property color primary: "#BB86FC"         // 按钮、高亮元素
    readonly property color secondary:"#03DAC6"        // 次要操作、图标
    readonly property color textPrimary: "#E0E0E0"     // 正文/标题
    readonly property color textSecondary: "#9E9E9E"   // 描述性文本
    readonly property color warning: "#CF6679"         // 错误提示/警告
    readonly property color primaryBorder: "#4C4C4C"   // 主界面边界
    readonly property color secondaryBorder: "#2A2A2A" // 次界面边界
}
