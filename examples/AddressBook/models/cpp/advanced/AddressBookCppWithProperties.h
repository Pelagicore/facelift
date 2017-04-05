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
        m_isLoaded = true;
        setImplementationID("C++ model implemented with properties");
    }

    void selectContact(int contactId) override {
        auto* contact = m_contacts.elementPointerById(contactId);
        if (contact != nullptr)
            m_currentContact = *contact;
        else
            qWarning() << "Unknown elementID " << contactId;
    }

    void createNewContact() override {
        if (m_contacts.list().size() < 3) {
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

};

