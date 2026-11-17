import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Dialog {
    id: root

    required property AccountsModel accounts
    required property var record
    property bool submitted: false

    Material.elevation: 0
    anchors.centerIn: parent
    height: Math.min(implicitHeight, parent ? parent.height - 32 : implicitHeight)
    modal: true
    parent: Overlay.overlay
    title: qsTr("Настройки %1").arg(root.record.tag)
    width: Math.min(500, parent ? parent.width - 32 : 500)

    contentItem: ScrollView {
        id: scroll

        clip: true
        implicitHeight: form.implicitHeight

        ColumnLayout {
            id: form

            width: scroll.availableWidth

            Label {
                text: qsTr("Псевдоним (необязательно)")
                textFormat: Text.PlainText
            }
            TextField {
                id: aliasInput

                Accessible.name: qsTr("Псевдоним")
                Layout.fillWidth: true
                text: root.record.alias
            }
            Label {
                text: qsTr("Клан обычной КВ; пусто — текущий клан")
                textFormat: Text.PlainText
            }
            TextField {
                id: warInput

                Accessible.name: qsTr("Клан КВ")
                Layout.fillWidth: true
                placeholderText: qsTr("#TAG")
                text: root.record.warClan
            }
            Label {
                text: qsTr("Клан участия в ЛВК")
                textFormat: Text.PlainText
            }
            TextField {
                id: cwlInput

                Accessible.name: qsTr("Клан ЛВК")
                Layout.fillWidth: true
                placeholderText: qsTr("#TAG")
                text: root.record.cwlClan
            }
            Label {
                text: qsTr("Планируемый переход; пусто — отменить план")
                textFormat: Text.PlainText
            }
            TextField {
                id: targetInput

                Accessible.name: qsTr("Целевой клан")
                Layout.fillWidth: true
                placeholderText: qsTr("#TAG")
                text: root.record.transferClan
            }
            Label {
                text: qsTr("Местное время перехода (необязательно)")
                textFormat: Text.PlainText
            }
            TextField {
                id: timeInput

                Accessible.name: qsTr("Время перехода")
                Layout.fillWidth: true
                placeholderText: qsTr("ГГГГ-ММ-ДДTчч:мм")
                text: root.record.transferAt
            }
            Label {
                Layout.fillWidth: true
                text: root.accounts.error
                textFormat: Text.PlainText
                visible: root.accounts.error.length > 0
                wrapMode: Text.Wrap
            }
            Label {
                Layout.fillWidth: true
                text: qsTr("Переход выполняется вручную в игре. Здесь сохраняется только план.")
                textFormat: Text.PlainText
                wrapMode: Text.Wrap
            }
        }
    }
    footer: RowLayout {
        Item {
            Layout.fillWidth: true
        }
        Button {
            text: qsTr("Отмена")

            onClicked: root.close()
        }
        Button {
            enabled: !root.accounts.saving
            text: qsTr("Сохранить")

            onClicked: {
                root.submitted = root.accounts.updateAccount(root.record.tag, aliasInput.text, warInput.text, cwlInput.text, targetInput.text, timeInput.text);
            }
        }
    }

    Connections {
        function onAccountsCommitted() {
            if (root.submitted)
                root.close();
        }

        target: root.accounts
    }
}
