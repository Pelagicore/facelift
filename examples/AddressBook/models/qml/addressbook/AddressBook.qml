/*
 *   This file is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the <license name> licence.
 *   Please see the LICENSE file for details. 
 */

import QtQuick 2.0
import QtQml 2.2
import facelift.example.addressbook 1.0

import QtQuick.Controls 1.2

/**
 * QML Implementation of the AddressBook model API
 */
AddressBookQMLImplementation {
    id: root

    isLoaded: false
    implementationID: "QML model"

    property string privateProperty: "This property is not defined in the public interface, but accessible via the \"provider\" property"

    currentContact: firstContact

    contacts: [ firstContact ]

    Timer {
        interval: 4000
        running: true
        onTriggered: {
            root.isLoaded = true
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
        var contactList = root.contacts;
        var foundIndex = findContactForId(contactId);
        if (foundIndex != -1) {
            currentContact = contactList[foundIndex];
            print("Selected contact with ID " + contactId);
        }
    }

    property int nextContactIndex: 1

    createNewContact: function() {
        var contactList = root.contacts;
        if (contactList.length < 5) {
            var newContact = ContactFactory.create();
            newContact.name = "New contact " + nextContactIndex++;
            newContact.number = "089 4892";
            contactList.push(newContact);
            root.contacts = contactList;
            contactCreated(newContact);
            currentContact = contact;
        }
        else {
            contactCreationFailed(FailureReason.Full);
        }
    }

    updateContact: function() {
        print("Not implemented"); // TODO
    }

    updateContact: function(contactId, contact) {
        var contactList = root.contacts;
        var foundIndex = findContactForId(contactId);
        if (foundIndex != -1) {
            contact.id = contactId;
            contactList[foundIndex] = contact;
            contacts = contactList;
        }
    }

    deleteContact: function(contactId) {
        var contactList = root.contacts;
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
    
}
