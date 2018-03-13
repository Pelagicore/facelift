import QtQuick 2.5
import QtQuick.Layouts 1.2

/**
 * Media player application UI
 */

Item {

    id: root

    width: 700
    height: 600

    property var advancedModel

    ListView {
        id: mediaList
        Layout.fillHeight: true
        width: root.width / 3
        height: 400

        model: advancedModel.theModel

        delegate: Rectangle {
            height: 30
            width: parent.width
            color: (index % 2) == 0 ? "gray" : "lightgray"

            Row {
            
                spacing: 10
                anchors.fill: parent
            
                Rectangle {
                    width: 50
                    height: parent.height 
                    color: "blue"
                    
                    Text {
                        text: "Remove"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            advancedModel.deleteModelItem(modelData)
                        }
                    }
                    
                }
    
                Text {
                    id: trackNameLabel
                    text: modelData.name
                }

                Rectangle {
                    width: 50
                    height: parent.height 
                    color: "red"

                    Text {
                        text: "Rename"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            advancedModel.renameModelItem(modelData, "renamed item")
                        }
                    }

                }

                Rectangle {
                    width: 50
                    height: parent.height
                    color: "green"
                    
                    Text {
                        text: "Insert"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            advancedModel.insertNewModelItemAfter(modelData)
                        }
                    }

                }

            }

        }
    }
}
