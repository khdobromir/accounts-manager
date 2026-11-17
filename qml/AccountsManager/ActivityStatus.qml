import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Frame {
    id: root

    required property var activity
    required property string title

    ColumnLayout {
        width: root.availableWidth

        Label {
            font.bold: true
            text: root.title
            textFormat: Text.PlainText
        }
        Label {
            Layout.fillWidth: true
            font.bold: root.activity.attention
            text: root.activity.status
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
        }
        Label {
            Layout.fillWidth: true
            text: root.activity.detail
            textFormat: Text.PlainText
            visible: root.activity.detail.length > 0
            wrapMode: Text.Wrap
        }
        Label {
            Layout.fillWidth: true
            text: qsTr("Окончание: %1").arg(root.activity.end)
            textFormat: Text.PlainText
            visible: root.activity.end.length > 0
            wrapMode: Text.Wrap
        }
        Label {
            Layout.fillWidth: true
            font.pixelSize: 12
            text: qsTr("%1 · %2").arg(root.activity.source).arg(root.activity.updated)
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
        }
        Label {
            color: "#ffcf80"
            text: root.activity.loading ? qsTr("Обновляется…") : qsTr("Сохранённые данные устарели")
            textFormat: Text.PlainText
            visible: root.activity.loading || root.activity.stale
        }
        Label {
            Layout.fillWidth: true
            color: "#ffcf80"
            text: root.activity.error
            textFormat: Text.PlainText
            visible: root.activity.error.length > 0
            wrapMode: Text.Wrap
        }
    }
}
