import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CSComputePipelineTest

ApplicationWindow {
    width: 1100
    height: 760
    visible: true
    title: qsTr("ComputePipeline Tests")
    color: "#202124"

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12

            Label {
                text: qsTr("ComputePipeline")
                font.bold: true
            }
            ComboBox {
                id: caseSelector
                model: [qsTr("Edge-preserving filter"), qsTr("Laplacian smoothing")]
                currentIndex: testController.currentCase
                onActivated: testController.currentCase = currentIndex
            }
            Item { Layout.fillWidth: true }
            BusyIndicator { running: testController.busy; visible: running; implicitWidth: 28; implicitHeight: 28 }
            Button { text: qsTr("Run Again"); enabled: !testController.busy; onClicked: testController.requestRun() }
            Button { text: qsTr("Save PNG"); enabled: testController.hasResult && !testController.busy; onClicked: testController.saveResult() }
        }
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 16
        color: "#151618"
        border.color: "#3c4043"
        radius: 4

        ComputeResultItem {
            anchors.fill: parent
            anchors.margins: 1
            controller: testController
        }
    }

    footer: ToolBar {
        Label {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 12
            text: testController.statusText
        }
    }
}
