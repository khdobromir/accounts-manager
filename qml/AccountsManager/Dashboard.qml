pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

Pane {
    id: root

    required property AccountsModel accounts
    required property OmarchyTheme theme

    ListView {
        id: list

        anchors.fill: parent
        clip: true
        model: root.accounts
        spacing: 12

        ScrollBar.vertical: ScrollBar {
        }
        delegate: AccountCard {
            theme: root.theme
            width: list.width
        }
    }
    Label {
        anchors.centerIn: parent
        horizontalAlignment: Text.AlignHCenter
        text: qsTr("Добавьте аккаунты на вкладке «Аккаунты». Нужен только публичный тег игрока.")
        textFormat: Text.PlainText
        visible: root.accounts.ready && root.accounts.count === 0
        width: parent.width - 32
        wrapMode: Text.Wrap
    }
}
