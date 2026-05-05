import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CSEditor.Theme

// Renders a single component inside the inspector.
// The C++ QuickComponentModel exposes roles: uiName, typeTag, value, writable.
// typeTag values match CSEditor::PropertyType:
//   0=Unknown 1=Bool 2=Int 3=Float 4=Double 5=String 6=Vector3
Rectangle {
    id: root
    color: CSTheme.surface
    radius: 4
    border.color: CSTheme.secondaryBorder
    border.width: 1
    implicitHeight: layout.implicitHeight + 10

    property var componentModel: null
    property bool expanded: true
    // Lower-case substring filter from the Inspector toolbar. Empty means "no filter".
    property string filter: ""

    // Hide the whole component if neither its name nor any of its property
    // names match the current filter.
    readonly property bool componentNameMatches: filter.length === 0
        || (componentModel && componentModel.componentName.toLowerCase().indexOf(filter) !== -1)

    function applyExpand(state) { expanded = state }

    ColumnLayout {
        id: layout
        anchors.fill: parent
        anchors.margins: 5
        spacing: 4

        // Collapsible header
        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Image {
                source: root.expanded
                    ? "qrc:/CSQML/qml/icons/hierarchy/cs_hierarchy_expanded.png"
                    : "qrc:/CSQML/qml/icons/hierarchy/cs_hierarchy_collapsed.png"
                sourceSize.width: 14
                sourceSize.height: 14
                fillMode: Image.PreserveAspectFit
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
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

                // Hide properties that don't match the filter. When hidden,
                // also collapse the row height so it doesn't leave a gap.
                readonly property bool rowVisible: root.filter.length === 0
                    || root.componentNameMatches
                    || uiNameText.toLowerCase().indexOf(root.filter) !== -1
                visible: rowVisible
                height: rowVisible ? implicitHeight : 0
                Layout.fillWidth: true
                Layout.preferredHeight: rowVisible ? implicitHeight : 0

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
                        default: return readonlyEditor
                        }
                    }
                }
            }
        }
    }

    // --- Editor components ---
    //
    // NOTE on binding hygiene: QML bindings to widget state (TextField.text,
    // CheckBox.checked, SpinBox.value) are destroyed the moment the user
    // interacts with the widget. We cannot attach `onEditorValueChanged`
    // handlers here because `editorValue` is a Loader-contextual property,
    // not a property of the editor root item (trying to do so yields
    // "Cannot assign to non-existent property onEditorValueChanged").
    // Instead we mirror it into a local `committed` property whose binding
    // automatically re-evaluates when `editorValue` changes, then push
    // `committed` onto the widget via `onCommittedChanged`, and use
    // `Qt.binding()` to re-establish the widget binding after user input.

    Component {
        id: boolEditor
        CheckBox {
            id: boolCtrl
            property bool committed: editorValue === true
            checked: committed
            enabled: editorWritable
            onCommittedChanged: if (checked !== committed) checked = committed
            onToggled: {
                if (root.componentModel) {
                    root.componentModel.setValue(editorRow, checked)
                }
                // Re-establish binding broken by user interaction.
                checked = Qt.binding(function() { return boolCtrl.committed })
            }
        }
    }

    Component {
        id: intEditor
        SpinBox {
            id: intCtrl
            from: -1000000
            to: 1000000
            property int committed: (typeof editorValue === "number") ? editorValue : 0
            value: committed
            enabled: editorWritable
            onCommittedChanged: if (value !== committed) value = committed
            onValueModified: {
                if (root.componentModel) root.componentModel.setValue(editorRow, value)
                // Re-establish binding broken by user interaction.
                value = Qt.binding(function() { return intCtrl.committed })
            }
        }
    }

    Component {
        id: floatEditor
        TextField {
            id: floatCtrl
            property real committed: (editorValue !== undefined && editorValue !== null) ? Number(editorValue) : 0
            text: committed.toFixed(4)
            enabled: editorWritable
            selectByMouse: true
            validator: DoubleValidator { notation: DoubleValidator.StandardNotation }
            onCommittedChanged: if (!activeFocus) text = committed.toFixed(4)
            onEditingFinished: {
                if (!root.componentModel) return
                var num = parseFloat(text)
                if (!isNaN(num)) {
                    root.componentModel.setValue(editorRow, num)
                }
                // Re-establish text binding broken by user typing.
                text = Qt.binding(function() { return floatCtrl.committed.toFixed(4) })
            }
        }
    }

    Component {
        id: stringEditor
        TextField {
            id: strCtrl
            property string committed: (editorValue !== undefined && editorValue !== null) ? editorValue : ""
            text: committed
            enabled: editorWritable
            selectByMouse: true
            onCommittedChanged: if (!activeFocus) text = committed
            onEditingFinished: {
                if (!root.componentModel) return
                root.componentModel.setValue(editorRow, text)
                // Re-establish text binding broken by user typing.
                text = Qt.binding(function() { return strCtrl.committed })
            }
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
                    id: axisCtrl
                    Layout.fillWidth: true
                    Layout.preferredWidth: 10
                    // Local mirror of the axis component.
                    property real committed: {
                        var v = vecRoot.vecValue
                        return (v && v.length > index) ? Number(v[index]) : 0
                    }
                    text: committed.toFixed(3)
                    enabled: editorWritable
                    selectByMouse: true
                    validator: DoubleValidator { notation: DoubleValidator.StandardNotation }
                    // Re-sync from model whenever the vector changes and we
                    // are not actively being edited.
                    Connections {
                        target: vecRoot
                        function onVecValueChanged() {
                            var v = vecRoot.vecValue
                            var next = (v && v.length > index) ? Number(v[index]) : 0
                            if (!axisCtrl.activeFocus && axisCtrl.committed !== next) {
                                axisCtrl.committed = next
                                axisCtrl.text = next.toFixed(3)
                            }
                        }
                    }
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
                            committed = num
                            root.componentModel.setValue(editorRow, arr)
                        }
                    }
                }
            }
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
