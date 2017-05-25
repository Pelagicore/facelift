/*
 *   This file is part of the QMLCppAPI project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the <license name> licence.
 *   Please see the LICENSE file for details. 
 */

#pragma once

#include "addressbook/AddressBookPropertyAdapter.h"

using namespace addressbook;

/**
 * C++ Implementation of the AddressBook API, based on the Property template class
 */
class AddressBookCppWithProperties: public AddressBookPropertyAdapter {

    Q_OBJECT

public:
    AddressBookCppWithProperties(QObject* parent = nullptr) :
            AddressBookPropertyAdapter(parent) {
        setImplementationID("C++ model implemented with properties");

        m_isLoaded.bind([this] () {
    		return !(m_timer.remainingTime() > 0);
    	});
        m_timer.setSingleShot(true);
        m_timer.start(4000);
        connect(&m_timer, &QTimer::timeout, this, &AddressBook::isLoadedChanged);
    }

    void selectContact(int contactId) override {
        auto* contact = m_contacts.elementPointerById(contactId);
        if (contact != nullptr)
            m_currentContact = *contact;
        else
            qWarning() << "Unknown elementID " << contactId;
    }

    void updateContact(int contactId, Contact newContact) override {
        auto* contact = m_contacts.elementPointerById(contactId);

        Q_ASSERT(contact != nullptr);
        if (contact != nullptr) {
            *contact = newContact;
            contact->setId(contactId);
            qDebug() << "Updated contact " << newContact.toString();
            contactsChanged();
        }
        else
            qWarning() << "Unknown elementID " << newContact.id();
    }

    void deleteContact(int contactId) override {
        auto* contact = m_contacts.elementPointerById(contactId);

        Q_ASSERT(contact != nullptr);
        if (contact != nullptr) {

        	contactDeleted(*contact);
        	m_contacts.removeElementById(contactId);

        	// Select first contact from the list if it exists
        	if (m_contacts.size() > 0)
        	    selectContact(m_contacts.elementAt(0).id());
        	else
        		m_currentContact = Contact();

        	contactsChanged();
        }
        else
            qWarning() << "Unknown elementID " << contactId;
    }

    void createNewContact() override {
        if (m_contacts.list().size() < 10) {
            qDebug() << "C++ createNewContact called";

            static int nextContactIndex = 0;

            Contact newContact;
            newContact.setname("New contact " + QString::number(nextContactIndex));
            newContact.setnumber("089 " + QString::number(nextContactIndex));
            m_contacts.addElement(newContact);
            nextContactIndex++;

            // Set as current contact
            m_currentContact = newContact;

            contactCreated(newContact);
        }
        else {
            contactCreationFailed(FailureReason::Full);
        }

    }

    QTimer m_timer;

};

