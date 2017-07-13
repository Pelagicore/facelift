/*
 *   This file is part of the QMLCppAPI project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the <license name> licence.
 *   Please see the LICENSE file for details. 
 */

import QtQuick 2.0
import QtQml 2.2
import addressbook 1.0

import QtQuick.Controls 1.2

/**
 * QML Implementation of the AddressBook model API
 */
AddressBookImplementation {
    id: root

    isLoaded: false
    implementationID: "QML model"

    property string privateProperty: "This property is not defined in the public interface, but accessible via the \"provider\" property"

    currentContact: Contact {
        name: "------"
        number: "------"
    }

    Timer {
        interval: 4000
        running: true
        onTriggered: {
            isLoaded = true
        }
    }

    Contact {
        id: contact
        property int index: 0
        name: "New contact " + index
        number: "089 4892"
    }

    selectContact: function(contactId) {
        print("Select contact with ID " + contactId);
        if (contacts.elementExists(contactId)) {
            currentContact = contacts.elementById(contactId);
        }
    }

    createNewContact: function() {
        if (contacts.size() < 5) {
	        var newContact = contacts.addCloneOf(contact);
            contactCreated(newContact);
            currentContact = contacts.elementById(newContact.id);
	        contact.index++;
	    }
	    else {
            contactCreationFailed(FailureReason.Full);
	    }
    }

    testMethod: function(contact) {
       print("contact");
       contact.name = "Reeeeturn"
       return contact;
    }

    updateContact: function(contactID, contact) {
        print("Not implemented"); // TODO
    }

    deleteContact: function(contactID) {
        contacts.removeElementByID(contactID)
    }

}
