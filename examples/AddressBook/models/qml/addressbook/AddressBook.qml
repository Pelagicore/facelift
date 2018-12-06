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

import QtQuick 2.0
import QtQml 2.2
import facelift.example.addressbook 1.0

import QtQuick.Controls 1.2

/**
 * QML Implementation of the AddressBook model API
 */
AddressBookImplementationBase {
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
