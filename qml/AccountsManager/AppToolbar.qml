import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ToolBar {
    id: root

    required property AccountsModel accounts
    required property int currentPage

    signal navigate(int page)
    signal settingsRequested

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12

        TabBar {
            Layout.preferredWidth: 240
            currentIndex: root.currentPage

            onCurrentIndexChanged: root.navigate(currentIndex)

            TabButton {
                text: qsTr("Обзор")
            }
            TabButton {
                text: qsTr("Аккаунты")
            }
        }
        Item {
            Layout.fillWidth: true
        }
        BusyIndicator {
            implicitHeight: 32
            implicitWidth: 32
            running: visible && root.accounts.busy
            visible: root.accounts.busy
        }
        ToolButton {
            enabled: root.accounts.ready && !root.accounts.busy && !root.accounts.saving
            text: qsTr("Обновить")

            onClicked: root.accounts.refresh()
        }
        ToolButton {
            enabled: root.accounts.ready && !root.accounts.saving
            text: qsTr("Настройки")

            onClicked: root.settingsRequested()
        }
    }
}
