import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import QtWebKit 3.0

/**
 * Media player application UI
 */

Item {

    id: root

    width: 700
    height: 600

    property var mediaIndexerModel

    ListView {
        id: mediaList
        Layout.fillHeight: true
        width: root.width / 3
        height: 400

        model: mediaIndexerModel.files

        delegate: Rectangle {
            height: 30
            width: parent.width
            color: (index % 2) == 0 ? "gray" : "lightgray"

            Label {
                id: trackNameLabel
                text: modelData.title + ( modelData.artist ? (" - " + modelData.artist) : "")
            }

            MouseArea {
                anchors.fill: trackNameLabel
                onClicked: {
                    print("Clicked " + modelData.title)
                    webview.url = modelData.url;
                }
            }
        }
    }

    WebView {
        id: webview
        anchors.left: mediaList.right
        anchors.right: root.right

        width: 400
        height: 400
        z: -1
    }

}
