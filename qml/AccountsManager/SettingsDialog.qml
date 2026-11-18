import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Dialog {
    id: root

    required property AccountsModel accounts

    Material.elevation: 0
    anchors.centerIn: parent
    modal: true
    title: qsTr("Настройки обновления")
    width: Math.min(500, parent ? parent.width - 32 : 500)

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
                root.accounts.setRefreshMinutes(interval.value);
            }
        }
    }

    Connections {
        function onIntervalCommitted() {
            root.close();
        }

        target: root.accounts
    }
    ColumnLayout {
        width: root.availableWidth

        Label {
            Layout.fillWidth: true
            text: qsTr("Период в минутах; 0 — только вручную")
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
        }
        SpinBox {
            id: interval

            Accessible.name: qsTr("Период обновления")
            editable: true
            from: 0
            to: 1440
            value: root.accounts.refreshMinutes
        }
        Label {
            Layout.fillWidth: true
            text: qsTr("База: %1").arg(root.accounts.databasePath)
            textFormat: Text.PlainText
            wrapMode: Text.WrapAnywhere
        }
        Label {
            text: qsTr("Ranked показывает недельный остаток атак. Для Legend I API не сообщает личный лимит на игровые сутки.")
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
    }
}
