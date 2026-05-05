import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CSEditor.Theme

// Renders a single component inside the inspector.
// The C++ QuickComponentModel exposes roles: uiName, typeTag, value, writable.
// typeTag values match CSEditor::PropertyType:
//   0=Unknown 1=Bool 2=Int 3=Float 4=Double 5=String 6=Vector3 7=Matrix4
Rectangle {
    id: root
    color: CSTheme.surface
    radius: 4
    border.color: CSTheme.secondaryBorder
    border.width: 1
    implicitHeight: layout.implicitHeight + 10

    property var componentModel: null
    property bool expanded: true

    ColumnLayout {
        id: layout
        anchors.fill: parent
        anchors.margins: 5
        spacing: 4

        // Collapsible header
        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Label {
                text: root.expanded ? "\u25BC" : "\u25B6"
                color: CSTheme.textSecondary
                MouseArea {
                    anchors.fill: parent
                    onClicked: root.expanded = !root.expanded
                }
            }

            Label {
                Layout.fillWidth: true
                text: root.componentModel ? root.componentModel.componentName : ""
                color: CSTheme.textPrimary
                font.bold: true
                elide: Text.ElideRight
            }
        }

        // Property rows
        Repeater {
            model: root.expanded ? root.componentModel : null
            delegate: RowLayout {
                id: row
                Layout.fillWidth: true
                spacing: 8

                // Forward model roles into explicit delegate properties so
                // that inner editor components (which may have their own
                // `value` / `text` properties that shadow role names) can
                // access them unambiguously.
                readonly property int rowIndex: index
                readonly property int typeTagValue: model.typeTag
                readonly property bool isWritable: model.writable
                readonly property var modelValue: model.value
                readonly property string uiNameText: model.uiName

                Label {
                    Layout.preferredWidth: 100
                    Layout.minimumWidth: 80
                    text: row.uiNameText
                    color: CSTheme.textPrimary
                    elide: Text.ElideRight
                }

                Loader {
                    id: editorLoader
                    Layout.fillWidth: true

                    // Explicit context for the sourceComponent
                    property int editorRow: row.rowIndex
                    property var editorValue: row.modelValue
                    property bool editorWritable: row.isWritable

                    sourceComponent: {
                        switch (row.typeTagValue) {
                        case 1: return boolEditor
                        case 2: return intEditor
                        case 3:
                        case 4: return floatEditor
                        case 5: return stringEditor
                        case 6: return vector3Editor
                        case 7: return matrix4Editor
                        default: return readonlyEditor
                        }
                    }
                }
            }
        }
    }

    // --- Editor components ---

    Component {
        id: boolEditor
        CheckBox {
            checked: editorValue === true
            enabled: editorWritable
            onToggled: if (root.componentModel) root.componentModel.setValue(editorRow, checked)
        }
    }

    Component {
        id: intEditor
        SpinBox {
            from: -1000000
            to: 1000000
            value: (typeof editorValue === "number") ? editorValue : 0
            enabled: editorWritable
            onValueModified: if (root.componentModel) root.componentModel.setValue(editorRow, value)
        }
    }

    Component {
        id: floatEditor
        TextField {
            text: (editorValue !== undefined && editorValue !== null) ? Number(editorValue).toFixed(4) : "0"
            enabled: editorWritable
            selectByMouse: true
            validator: DoubleValidator { notation: DoubleValidator.StandardNotation }
            onEditingFinished: {
                if (root.componentModel) {
                    var num = parseFloat(text)
                    if (!isNaN(num)) root.componentModel.setValue(editorRow, num)
                }
            }
        }
    }

    Component {
        id: stringEditor
        TextField {
            text: (editorValue !== undefined && editorValue !== null) ? editorValue : ""
            enabled: editorWritable
            selectByMouse: true
            onEditingFinished: if (root.componentModel) root.componentModel.setValue(editorRow, text)
        }
    }

    Component {
        id: vector3Editor
        RowLayout {
            id: vecRoot
            spacing: 4
            property var vecValue: editorValue
            Repeater {
                model: 3
                delegate: TextField {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 10
                    text: {
                        var v = vecRoot.vecValue
                        return (v && v.length > index) ? Number(v[index]).toFixed(3) : "0"
                    }
                    enabled: editorWritable
                    selectByMouse: true
                    validator: DoubleValidator { notation: DoubleValidator.StandardNotation }
                    onEditingFinished: {
                        if (!root.componentModel) return
                        var src = vecRoot.vecValue
                        var arr = [
                            (src && src.length > 0) ? Number(src[0]) : 0,
                            (src && src.length > 1) ? Number(src[1]) : 0,
                            (src && src.length > 2) ? Number(src[2]) : 0
                        ]
                        var num = parseFloat(text)
                        if (!isNaN(num)) {
                            arr[index] = num
                            root.componentModel.setValue(editorRow, arr)
                        }
                    }
                }
            }
        }
    }

    Component {
        id: matrix4Editor
        Label {
            text: (editorValue !== undefined && editorValue !== null) ? editorValue : ""
            color: CSTheme.textSecondary
            font.family: "Consolas, monospace"
            wrapMode: Text.NoWrap
            elide: Text.ElideRight
        }
    }

    Component {
        id: readonlyEditor
        Label {
            text: (editorValue !== undefined && editorValue !== null) ? String(editorValue) : "<unknown>"
            color: CSTheme.textSecondary
            elide: Text.ElideRight
        }
    }
}
