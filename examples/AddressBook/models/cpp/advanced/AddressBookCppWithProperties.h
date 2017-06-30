/*
 *   This file is part of the QMLCppAPI project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the <license name> licence.
 *   Please see the LICENSE file for details.
 */

#pragma once

#include "addressbook/AddressBookPropertyAdapter.h"
#include "addressbook/SubInterfacePropertyAdapter.h"


using namespace addressbook;

/**
 * C++ Implementation of the AddressBook API, using the PropertyAdapter helper class
 */
class AddressBookCppWithProperties :
    public AddressBookPropertyAdapter
{

    Q_OBJECT

public:
    // This property is not defined as part of the public interface (IDL), but it can be accessed via the "provider" property
    Q_PROPERTY(QString privateProperty READ privateProperty CONSTANT)
    QString privateProperty() const
    {
        return "This property is a private property of the interface implementation, accessible via the \"provider\" property";
    }

    // This method is not defined as part of the public interface (IDL), but it can be called via the "provider" property
    Q_INVOKABLE void privateMethod() {
    	qDebug() << "privateMethod called !";
    }

    class SubInterfaceImpl :
        public SubInterfacePropertyAdapter
    {
        void doSomething() override
        {
            qDebug() << "doSomething() called";
        }
    };

    SubInterfaceImpl m_subInterface;


    AddressBookCppWithProperties(QObject *parent = nullptr) :
        AddressBookPropertyAdapter(parent)
    {
        setImplementationID("C++ model implemented with properties");

        m_timer.setSingleShot(true);
        m_timer.start(3000);

        m_isLoaded.bind([this] () {
            return !(m_timer.remainingTime() > 0);
        }).addTrigger(&m_timer, &QTimer::timeout);

        m_subService = &m_subInterface;
    }

    void selectContact(int contactId) override
    {
        auto *contact = m_contacts.elementPointerById(contactId);
        if (contact != nullptr) {
            m_currentContact = *contact;
        } else {
            qWarning() << "Unknown elementID " << contactId;
        }
    }

    void updateContact(int contactId, Contact newContact) override
    {
        bool found = false;
        auto list = m_contacts.value();
        for (auto &contact : list) {
            if (contact.id() == contactId) {
                contact = newContact;
                contact.setId(contactId);
                qDebug() << "Updated contact " << contact.toString();
                found = true;
            }
        }

        if (found) {
            // Assign the new list
            m_contacts = list;
        } else {
            qWarning() << "Unknown elementID " << newContact.id();
        }
    }

    void deleteContact(int contactId) override
    {
        auto *contact = m_contacts.elementPointerById(contactId);

        if (contact != nullptr) {

            contactDeleted(*contact);
            m_contacts.removeElementById(contactId);

            // Select first contact from the list if it exists
            if (m_contacts.size() > 0) {
                selectContact(m_contacts.value()[0].id());
            } else {
                m_currentContact = Contact();
            }

            contactsChanged();
        } else {
            qWarning() << "Unknown elementID " << contactId;
        }
    }

    void createNewContact() override
    {
        if (m_contacts.size() < 10) {
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
        } else {
            contactCreationFailed(FailureReason::Full);
        }

    }

    QTimer m_timer;

};
