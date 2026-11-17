import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Frame {
    id: root

    readonly property var account: model.account
    required property var model

    ColumnLayout {
        spacing: 10
        width: root.availableWidth

        RowLayout {
            Layout.fillWidth: true

            Label {
                Layout.fillWidth: true
                font.bold: true
                font.pixelSize: 19
                text: qsTr("%1 · %2").arg(root.account.name).arg(root.account.tag)
                textFormat: Text.PlainText
                wrapMode: Text.Wrap
            }
            Label {
                color: "#ffcf80"
                text: qsTr("Требует внимания")
                textFormat: Text.PlainText
                visible: root.account.attention
            }
        }
        Label {
            Layout.fillWidth: true
            text: qsTr("Текущий клан: %1").arg(root.account.clan)
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
        }
        Label {
            Layout.fillWidth: true
            color: "#ffcf80"
            text: root.account.action
            textFormat: Text.PlainText
            visible: root.account.action.length > 0
            wrapMode: Text.Wrap
        }
        GridLayout {
            Layout.fillWidth: true
            columnSpacing: 10
            columns: root.width >= 850 ? 3 : 1
            rowSpacing: 10

            ActivityStatus {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                activity: root.account.war
                title: qsTr("КВ · %1").arg(root.account.warClan || qsTr("клан не задан"))
            }
            ActivityStatus {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                activity: root.account.cwl
                title: qsTr("ЛВК · %1").arg(root.account.cwlClan || qsTr("клан не задан"))
            }
            ActivityStatus {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                activity: root.account.ranked
                title: root.account.league ? qsTr("Ranked · %1").arg(root.account.league) : qsTr("Ranked")
            }
        }
    }
}
