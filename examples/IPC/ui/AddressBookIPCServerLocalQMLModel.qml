import QtQuick 2.0
import facelift.example.addressbook 1.0

import QtQuick.Controls 1.2

import "../../AddressBook/ui"
import "../../AddressBook/models/qml/addressbook"

Item {

    id: root
    height: mainPanel.height
    width: mainPanel.width

    Rectangle {
        anchors.fill: parent
	    color: "green"
    }

    AddressBookMainPanel {
        id: mainPanel
    	anchors.fill: parent
    	viewModel: proxy
    }

    AddressBookIPCProxy {
        id: proxy
    }

    AddressBookQMLImplementation {
        id: impl

        isLoaded: false
        implementationID: "QML model"

        property string privateProperty: "This property is not defined in the public interface, but accessible via the \"provider\" property"

        currentContact: firstContact

        contacts: [ firstContact ]

        Timer {
            interval: 4000
            running: true
            onTriggered: {
                impl.isLoaded = true
            }
        }

        Contact {
            id: firstContact
            name: "New contact 0"
            number: "089 4892"
        }

        function findContactForId(contactId) {
            var contactList = contacts;
            for (var i = 0, len = contactList.length; i < len; i++) {
                if (contactList[i].id == contactId) {
                    return  i;
                }
            }
            print("Contact not found ID: " + contactId);
            return -1;
        }

        selectContact: function(contactId) {
            var contactList = impl.contacts;
            var foundIndex = findContactForId(contactId);
            if (foundIndex != -1) {
                currentContact = contactList[foundIndex];
                print("Selected contact with ID " + contactId);
            }
        }

        property int nextContactIndex: 1

        createNewContact: function() {
            var contactList = impl.contacts;
            if (contactList.length < 5) {
                var newContact = ContactFactory.create();
                newContact.name = "New contact " + nextContactIndex++;
                newContact.number = "089 4892";
                contactList.push(newContact);
                impl.contacts = contactList;
                contactCreated(newContact);
                currentContact = contact;
            }
            else {
                contactCreationFailed(FailureReason.Full);
            }
        }

        updateContact: function(contactId, contact) {
            print("Not implemented"); // TODO
        }

        deleteContact: function(contactId) {
            var contactList = impl.contacts;
            var foundIndex = findContactForId(contactId);

            if (foundIndex != -1) {
                contactList.splice(foundIndex, 1);
                print(contactList)
                contactDeleted(contactList[foundIndex]);
                contacts = contactList;
                selectContact(contactList[0].id);
            }
            else {
                print("Contact not found ID: " + contactId); // TODO
            }

        }

        IPC.enabled: true
    }

}
