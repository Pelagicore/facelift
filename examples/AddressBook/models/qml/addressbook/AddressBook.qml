import QtQuick 2.0
import QtQml 2.2
import qface.addressbook 1.0

import QtQuick.Controls 1.2

/**
 * QML Implementation of the AddressBook model API
 */
AddressBookImplementation {

    isLoaded: false
    implementationID: "QML model"

    property int nextIndex: 0

    property string privateProperty: "This property is not defined in the public interface, but accessible via the \"provider\" property"

    Timer {
        interval: 4000;
        running: true;
        repeat: false
        onTriggered: {
            isLoaded = true
        }
    }

    selectContact: function(contactId) {
        print("Select contact with ID " + contactId);
        if (contacts.elementExists(contactId))
            currentContact = contacts.elementById(contactId);
    }

    createNewContact: function() {
        if (contacts.size() < 5) {
	        var contact = AddressbookModule.createContact();
	        contact.name = "New contact " + nextIndex++ ;
	        contact.number = "089 4892";
	        var elementID = contacts.addElement(contact);
            contactCreated(contact);
	    }
	    else {
            contactCreationFailed(FailureReason.Full);
	    } 
    }

}
