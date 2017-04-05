import QtQuick 2.0
import qface.addressbook 1.0

import QtQuick.Controls 1.2

/**
 * AddressBook application UI
 */
Item {
    id: root
    
    width: 600
    height: 600

    property bool popupVisible: false
    property bool serverSide: true
    property var viewModel

    Timer {
        id: hidePopupTimer
        interval: 2000
        onTriggered: {
            popupVisible = false
        }
    }

    Text {
        id: implementationLabel
        text: viewModel.implementationID
    }

    Text {
        anchors.top: implementationLabel.bottom
        text: viewModel.provider.privateProperty
    }

    Item {

        anchors.fill: parent
        anchors.margins: 40

        TableView {
            id: tableView
            width: parent.width / 2
            height: parent.height
            anchors.left: parent.left
            headerVisible : true

            TableViewColumn {
                id: titleColumn
                title: "Contacts"
                role: "name"
                width: tableView.viewport.width
            }

            function updateSelection() {
                var contactID = viewModel.currentContact.id;
                selection.clear()
                for (var contactIndex in model) {
                    if (model[contactIndex].id === contactID) {
                        selection.select(contactIndex)
                    }
                }
            }

            onModelChanged: {
                updateSelection()
            }

            model: viewModel.contacts
        }

        Connections {
            target: tableView.selection
            onSelectionChanged: tableView.selection.forEach(function(rowIndex) {
                var elementID = tableView.model[rowIndex].id;
                viewModel.selectContact(elementID);
            });
        }

        Rectangle {
            width: parent.width/2
            height: parent.height
            anchors.right: parent.right
            color: "darkCyan"
            Column {

                width: parent.width

                Button {
                    height: 50
                    width: parent.width

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                        	viewModel.createNewContact()
                        }
                    }

                    Text {
                        anchors.fill: parent
                        text: "Create new contact"
                    }
                }

                spacing: 10
                Text {
                    text: "Name : " + viewModel.currentContact.name
                }

                Text {
                    text: "Number : " + viewModel.currentContact.number
                }
                
                Text {
                    text: "Family : " + ((viewModel.currentContact.type == ContactType.Family) ? "Yes" : "No")
                }
            }

        }

        BusyIndicator {
            anchors.margins: 200
            anchors.fill: parent
            running: !viewModel.isLoaded
        }
    }

    Rectangle {
        color: "lightgray"
        width: 300
        height:  100
        anchors.centerIn: parent
        visible: opacity > 0

        opacity: root.popupVisible ? 1 : 0
        Behavior on opacity {
            NumberAnimation {
                duration: 200
            }
        }

        Text {
            id: popupText
            anchors.centerIn: parent
        }
    }
    
    Connections {
    	target: viewModel

        onContactCreated : {
            popupText.text = "Contact created !"
            popupVisible = true;
            hidePopupTimer.restart();
        }

        onContactCreationFailed : {
            popupText.text = "Contact creation failed !\nReason : " + ( (reason==FailureReason.Full) ? "Full" : "Other")
            popupVisible = true;
            hidePopupTimer.restart();
        }

        onCurrentContactChanged: {
            tableView.updateSelection();
        }

    }

}

