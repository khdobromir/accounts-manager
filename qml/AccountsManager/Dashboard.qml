pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

Pane {
    id: root

    required property AccountsModel accounts
    required property OmarchyTheme theme

    AttentionModel {
        id: attention
        sourceModel: root.accounts
    }
    ComboBox {
        id: filter
        model: [qsTr("Все"), qsTr("Требуют внимания")]
        Accessible.name: qsTr("Фильтр аккаунтов")
    }
    ListView {
        id: list

        anchors {
            top: filter.bottom
            topMargin: 8
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        clip: true
        model: filter.currentIndex === 0 ? root.accounts : attention
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
        text: root.accounts.count > 0 ? qsTr("Сейчас ничего делать не нужно") : qsTr("Добавьте аккаунты на вкладке «Аккаунты». Нужен только публичный тег игрока.")
        textFormat: Text.PlainText
        visible: root.accounts.ready && list.count === 0
        width: parent.width - 32
        wrapMode: Text.Wrap
    }
}
