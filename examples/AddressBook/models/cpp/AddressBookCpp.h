/*
 *   This file is part of the QMLCppAPI project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the <license name> licence.
 *   Please see the LICENSE file for details.
 */

#pragma once

#include "addressbook/AddressBook.h"

using namespace addressbook;

/**
 * C++ Implementation of the AddressBook API
 */
class AddressBookCpp :
    public AddressBook
{

    Q_OBJECT

public:
    AddressBookCpp(QObject *parent = nullptr) :
        AddressBook(parent)
    {
        setImplementationID("C++ model");
    }

    Q_PROPERTY(QString privateProperty READ privateProperty CONSTANT)
    QString privateProperty() const
    {
        return "This property is a private property of the interface implementation, accessible via the \"provider\" property";
    }

    void createNewContact() override
    {
        qDebug() << "C++ createNewContact called";

        static int nextContactIndex = 0;

        Contact newContact;
        newContact.setname("New contact " + QString::number(nextContactIndex));
        newContact.setnumber("089 " + QString::number(nextContactIndex));
        m_contacts.append(newContact);
        contactsChanged();
        nextContactIndex++;

        m_currentContact = newContact;
        currentContactChanged();

        contactCreated(newContact);
    }

    void updateContact(int contactId, Contact newContact) override
    {
        Contact *contact = nullptr;

        for (auto &c : m_contacts) {
            if (c.id() == contactId) {
                contact = &c;
            }
        }

        Q_ASSERT(contact != nullptr);
        if (contact != nullptr) {
            *contact = newContact;
            contact->setId(contactId);
            qDebug() << "Updated contact " << newContact.toString();
            selectContact(contactId);
            contactsChanged();
        } else {
            qWarning() << "Unknown elementID " << newContact.id();
        }
    }

    void deleteContact(int contactId) override
    {
        Q_UNUSED(contactId);
        qWarning() << "TODO";
    }

    void selectContact(int contactId) override
    {
        if (m_currentContact.id() != contactId) {
            for (auto &contact : m_contacts) {
                if (contactId == contact.id()) {
                    m_currentContact = contact;
                    currentContactChanged();
                    qDebug() << "Current contact changed to " << m_currentContact.toString();
                }
            }
        }
    }

    const bool &isLoaded() const override
    {
        return m_isLoaded;
    }

    const Contact &currentContact() const override
    {
        return m_currentContact;
    }

    const QList<Contact> &contacts() const override
    {
        return m_contacts;
    }

private:
    Contact m_currentContact;
    bool m_isLoaded = true;
    QList<Contact> m_contacts;

};
