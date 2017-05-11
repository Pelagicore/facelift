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
class AddressBookCpp: public AddressBook {

    Q_OBJECT

//    class ContactsModelListModel : public ModelListModel {
//    public:
//
//        QHash<int,QByteArray> roleNames() const override {
//            return Contact::roleNames();
//        }
//
//        QVariant data(const QModelIndex &index, int role) const override {
//            return m_list[index.row()].getFieldAsVariant(role);
//        }
//
//        int rowCount(const QModelIndex &index) const override {
//            Q_UNUSED(index);
//            return m_list.size();
//        }
//
//        void addContact(const Contact& contact) {
//            beginResetModel();
//            m_list.append(contact);
//            endResetModel();
//        }
//
//        int elementID(int elementIndex) const override {
//            Q_ASSERT(elementIndex>=0);
//            Q_ASSERT(elementIndex<m_list.size());
//            return m_list[elementIndex].id();
//        }
//
//        QList<Contact> m_list;
//    };


public:
    AddressBookCpp(QObject* parent = nullptr) :
        AddressBook(parent) {
        setImplementationID("C++ model");
    }

    Q_PROPERTY(QString privateProperty READ privateProperty CONSTANT)
    QString privateProperty() const {
        return "This property is not defined in the public interface, but accessible via the \"provider\" property";
    }

    void createNewContact() override {
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

    void updateContact(int contactId, Contact newContact) override {
    	Contact* contact = nullptr;

    	for (auto& c : m_contacts) {
    		if (c.id() == contactId)
    			contact = &c;
    	}

        Q_ASSERT(contact != nullptr);
        if (contact != nullptr) {
            *contact = newContact;
            contact->setId(contactId);
            qDebug() << "Updated contact " << newContact.toString();
            currentContactChanged();
            contactsChanged();
        }
        else
            qWarning() << "Unknown elementID " << newContact.id();
    }

    void selectContact(int contactId) override {
        if (m_currentContact.id() != contactId) {
            for (auto& contact : m_contacts) {
                if(contactId == contact.id()) {
                    m_currentContact = contact;
                    currentContactChanged();
                }
            }
        }
    }

    bool isLoaded() const override {
        return m_isLoaded;
    }

    Contact currentContact() const override {
        return m_currentContact;
    }

    QList<Contact> contacts() const override {
        return m_contacts;
    }

private:
    Contact m_currentContact;
    bool m_isLoaded = true;
    QList<Contact> m_contacts;

};
