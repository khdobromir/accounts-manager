import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Frame {
    id: root

    readonly property var account: model.account
    required property var model
    required property OmarchyTheme theme

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
                color: root.theme.colors.yellow
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
            color: root.theme.colors.yellow
            text: root.account.action
            textFormat: Text.PlainText
            visible: root.account.action.length > 0
            wrapMode: Text.Wrap
        }
        Label {
            text: root.account.deadlineLabel
            textFormat: Text.PlainText
            visible: text.length > 0
            Layout.fillWidth: true
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
                theme: root.theme
                title: qsTr("КВ · %1").arg(root.account.warClan || qsTr("клан не задан"))
            }
            ActivityStatus {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                activity: root.account.cwl
                theme: root.theme
                title: qsTr("ЛВК · %1").arg(root.account.cwlClan || qsTr("клан не задан"))
            }
            ActivityStatus {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                activity: root.account.ranked
                theme: root.theme
                title: root.account.league ? qsTr("Ranked · %1").arg(root.account.league) : qsTr("Ranked")
            }
        }
    }
}
