import QtQuick 2.5
import facelift.example.addressbook 1.0

/**********************************************************************
**
** Copyright (C) 2018 Luxoft Sweden AB
**
** This file is part of the FaceLift project
**
** Permission is hereby granted, free of charge, to any person
** obtaining a copy of this software and associated documentation files
** (the "Software"), to deal in the Software without restriction,
** including without limitation the rights to use, copy, modify, merge,
** publish, distribute, sublicense, and/or sell copies of the Software,
** and to permit persons to whom the Software is furnished to do so,
** subject to the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
** BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
** ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
** SPDX-License-Identifier: MIT
**
**********************************************************************/

import QtQuick.Controls 1.4

/**
 * AddressBook application UI
 */

Item {

    id: root

    width: 700
    height: 600

    property bool popupVisible: false
    property bool serverSide: true
    property AddressBook viewModel
    property bool contactDirty: nameField.isDirty || numberField.isDirty

    Column {

        spacing: 10
        anchors.fill: parent

        Label {
            id: implementationLabel
            horizontalAlignment: Label.AlignHCenter
            text: viewModel.implementationID
        }

        Text {
            text: viewModel.provider.privateProperty
        }

        Item {

            anchors.margins: 40

            height: 300
            width: parent.width

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
                    if (viewModel.currentContact.id != elementID) {
                        viewModel.selectContact(elementID);
                    }
                });
            }

            Rectangle {

                width: parent.width / 2
                height: parent.height
                anchors.right: parent.right
                color: "darkCyan"

                Column {

                    width: parent.width
                    spacing: 10

                    StringField {
                        id: nameField
                        label: "Name"
                        text: viewModel.currentContact.name
                    }

                    StringField {
                        id: numberField
                        label: "Number"
                        text: viewModel.currentContact.number
                    }

                    Text {
                        text: "Family : " + ((viewModel.currentContact.type === ContactType.Family) ? "Yes" : "No")
                    }

                }

            }

        }

        Row {

            height: 100
            spacing: 20
            anchors.horizontalCenter: parent.Center

            Button {
                text: "Create new contact"
                onClicked: {
                    viewModel.createNewContact()
                }
            }

            Button {
                text: "Delete contact"
                onClicked: {
                    viewModel.deleteContact(viewModel.currentContact.id)
                }
            }

            Button {
                text: "Apply changes"
                enabled: contactDirty
                onClicked: {
                    var contact = viewModel.currentContact.clone();
                    contact.name = nameField.editedText;
                    contact.number = numberField.editedText;
                    viewModel.updateContact(viewModel.currentContact.id, contact)
                }
            }

            Button {
                text: "Discard changes"
                enabled: contactDirty
                onClicked: {
                    nameField.reset();
                    numberField.reset();
                }
            }

            Button {
                text: "Private method"
                onClicked: {
                    viewModel.provider.privateMethod();
                }
            }

            Button {
                text: "test method"
                onClicked: {
                    var contact = ContactFactory.create();
                    print(contact)
                    var returnContact = viewModel.testMethod(contact);
                    print("return value : " + returnContact)
                }
            }
        }

        Timer {
            id: hidePopupTimer
            interval: 2000
            onTriggered: {
                popupVisible = false
            }
        }

        Connections {
            target: viewModel

            onContactCreated : {
                showPopup("Contact created : " + contact.name)
            }

            onContactDeleted : {
                showPopup("Contact deleted : " + contact.name)
            }

            onContactCreationFailed : {
                showPopup("Contact creation failed !\nReason : " + ( (reason===FailureReason.Full) ? "Full" : "Other") )
            }

            onCurrentContactChanged: {
                tableView.updateSelection();
            }

        }

    }

    BusyIndicator {
        anchors.margins: 200
        anchors.fill: parent
        running: !viewModel.isLoaded
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

    Rectangle {
        anchors.fill: parent
        color: "red"
        visible: !viewModel.ready

        Label {
            anchors.centerIn: parent
            text: "Unavailable"
        }
    }

    function showPopup(text) {
        popupText.text = text
        popupVisible = true;
        hidePopupTimer.restart();
    }

}
