pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

ApplicationWindow {
    id: root

    required property AccountsModel accounts
    property int currentPage: 0
    property bool settingsOpen: false

    Material.theme: Material.Dark
    height: 800
    minimumHeight: 480
    minimumWidth: 540
    title: qsTr("AccountsManager")
    visible: true
    width: 1100

    header: AppToolbar {
        accounts: root.accounts
        currentPage: root.currentPage

        onNavigate: page => {
            root.currentPage = page;
        }
        onSettingsRequested: root.settingsOpen = true
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16

        Label {
            Layout.fillWidth: true
            color: "#ffcf80"
            text: root.accounts.error
            textFormat: Text.PlainText
            visible: root.accounts.error.length > 0
            wrapMode: Text.Wrap
        }
        Label {
            text: qsTr("Открытие локальной базы…")
            textFormat: Text.PlainText
            visible: !root.accounts.ready
        }
        StackLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            currentIndex: root.currentPage

            Dashboard {
                accounts: root.accounts
            }
            AccountsPage {
                accounts: root.accounts
            }
        }
    }
    Loader {
        active: root.settingsOpen

        sourceComponent: Component {
            SettingsDialog {
                accounts: root.accounts
                parent: Overlay.overlay

                onClosed: root.settingsOpen = false
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
