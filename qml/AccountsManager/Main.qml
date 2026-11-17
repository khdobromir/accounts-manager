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
    required property OmarchyTheme theme

    Material.accent: root.theme.accent
    Material.background: root.theme.background
    Material.foreground: root.theme.foreground
    Material.primary: root.theme.background
    Material.theme: root.theme.light ? Material.Light : Material.Dark
    color: root.theme.background
    height: 800
    minimumHeight: 480
    minimumWidth: 540
    palette.accent: root.theme.accent
    palette.base: root.theme.background
    palette.button: root.theme.background
    palette.buttonText: root.theme.foreground
    palette.highlight: root.theme.colors.selection
    palette.highlightedText: root.theme.selectionForeground
    palette.placeholderText: root.theme.colors.muted
    palette.text: root.theme.foreground
    palette.window: root.theme.background
    palette.windowText: root.theme.foreground
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
            color: root.theme.colors.red
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
                theme: root.theme
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
