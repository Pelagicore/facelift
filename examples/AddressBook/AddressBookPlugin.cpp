/*
 *   This file is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the <license name> licence.
 *   Please see the LICENSE file for details.
 */

#include "AddressBookPlugin.h"

#include "facelift/example/addressbook/Module.h"

#include "models/cpp/advanced/AddressBookCppWithProperties.h"
#include "models/cpp/AddressBookCpp.h"

#include "facelift/example/addressbook/AddressBookQMLImplementation.h"

using namespace facelift::example::addressbook;

void AddressBookPlugin::registerTypes(const char *uri)
{
    // Register the generated types
    Module::registerTypes();
    Module::registerQmlTypes(uri);

    AddressBookQMLImplementation::setModelImplementationFilePath(STRINGIFY(
                QML_MODEL_LOCATION) "/models/qml/addressbook/AddressBook.qml");
//    facelift::registerQmlComponent<AddressBookQMLImplementation::Provider>(uri);

    // We are registering the model types here, which can be used by the UI code.
    // The decision to register a dummy, QML, or C++ implementation should be taken here
        facelift::registerQmlComponent<AddressBookCppWithProperties>(uri);
    //    registerQmlComponent<AddressBookCpp>(uri);

}
