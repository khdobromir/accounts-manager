pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Pane {
    id: root

    required property AccountsModel accounts
    property bool adding: false
    property string editingTag: ""
    property string removingTag: ""

    Connections {
        function onAccountsCommitted() {
            if (root.adding) {
                tagInput.clear();
                root.adding = false;
            }
        }
        function onStateChanged() {
            if (!root.accounts.saving)
                root.adding = false;
        }

        target: root.accounts
    }
    ColumnLayout {
        anchors.fill: parent

        RowLayout {
            Layout.fillWidth: true

            TextField {
                id: tagInput

                Accessible.name: qsTr("Тег нового аккаунта")
                Layout.fillWidth: true
                enabled: root.accounts.ready && !root.accounts.saving
                placeholderText: qsTr("Публичный тег игрока: #TAG")

                onAccepted: add.click()
            }
            Button {
                id: add

                enabled: root.accounts.ready && !root.accounts.saving && tagInput.text.trim().length > 0
                text: qsTr("Добавить")

                onClicked: {
                    root.adding = root.accounts.addAccount(tagInput.text);
                }
            }
        }
        ListView {
            id: list

            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true
            model: root.accounts
            spacing: 10

            ScrollBar.vertical: ScrollBar {
            }
            delegate: Frame {
                id: entry

                required property int index
                required property var model

                width: list.width

                ColumnLayout {
                    width: entry.availableWidth

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("%1 · %2").arg(entry.model.account.name).arg(entry.model.account.tag)
                        textFormat: Text.PlainText
                        wrapMode: Text.Wrap
                    }
                    RowLayout {
                        enabled: !root.accounts.saving

                        Button {
                            text: qsTr("Настроить")

                            onClicked: root.editingTag = entry.model.account.tag
                        }
                        Button {
                            enabled: entry.index > 0
                            text: qsTr("Вверх")

                            onClicked: root.accounts.moveAccount(entry.model.account.tag, -1)
                        }
                        Button {
                            enabled: entry.index < root.accounts.count - 1
                            text: qsTr("Вниз")

                            onClicked: root.accounts.moveAccount(entry.model.account.tag, 1)
                        }
                        Button {
                            text: qsTr("Удалить")

                            onClicked: root.removingTag = entry.model.account.tag
                        }
                    }
                }
            }
        }
    }
    Loader {
        active: root.editingTag.length > 0

        sourceComponent: Component {
            AccountEditor {
                accounts: root.accounts
                parent: Overlay.overlay
                record: root.accounts.accountSettings(root.editingTag)

                onClosed: root.editingTag = ""
            }
        }

        onLoaded: {
            if (status === Loader.Ready) {
                const dialog = item as Dialog;
                if (dialog)
                    dialog.open();
            }
        }
    }
    Loader {
        active: root.removingTag.length > 0

        sourceComponent: Component {
            Dialog {
                Material.elevation: 0
                anchors.centerIn: parent
                modal: true
                parent: Overlay.overlay
                standardButtons: Dialog.Yes | Dialog.No
                title: qsTr("Удалить аккаунт?")

                onAccepted: root.accounts.removeAccount(root.removingTag)
                onClosed: root.removingTag = ""

                Label {
                    text: root.removingTag
                    textFormat: Text.PlainText
                }
            }
        }

        onLoaded: {
            if (status === Loader.Ready) {
                const dialog = item as Dialog;
                if (dialog)
                    dialog.open();
            }
        }
    }
}
