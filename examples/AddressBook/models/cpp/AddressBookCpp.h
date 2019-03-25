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

#pragma once

#include "facelift/example/addressbook/AddressBook.h"
#include "FaceliftLogging.h"


using namespace facelift::example::addressbook;

/**
 * C++ Implementation of the AddressBook API
 */
class AddressBookImplementation : public AddressBook
{
    Q_OBJECT

public:
    AddressBookImplementation(QObject *parent = nullptr) :
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
        qCDebug(LogGeneral) << "C++ createNewContact called";

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
            qCDebug(LogGeneral) << "Unknown elementID " << newContact.id();
        }
    }

    void deleteContact(int contactId) override
    {
        Q_UNUSED(contactId);
        qCDebug(LogGeneral) << "TODO";
    }

    void selectContact(int contactId) override
    {
        if (m_currentContact.id() != contactId) {
            for (auto &contact : m_contacts) {
                if (contactId == contact.id()) {
                    m_currentContact = contact;
                    currentContactChanged();
                    qCDebug(LogGeneral) << "Current contact changed to " << m_currentContact.toString();
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
