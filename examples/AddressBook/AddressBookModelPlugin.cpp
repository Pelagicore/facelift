/*
 *   This file is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the <license name> licence.
 *   Please see the LICENSE file for details.
 */

#include "AddressBookModelPlugin.h"

#include "addressbook/Module.h"
#include "addressbook/ModuleDummy.h"
#include "addressbook/ModuleMonitor.h"
#include "addressbook/AddressBookMonitor.h"

#ifdef FACELIFT_ENABLE_IPC
#include "addressbook/ModuleIPC.h"
#include "addressbook/AddressBookIPC.h"
#endif

#include "models/cpp/advanced/AddressBookCppWithProperties.h"
#include "models/cpp/AddressBookCpp.h"

using namespace addressbook;

void AddressBookModelPlugin::registerTypes(const char *uri)
{
    // Register the generated types
    Module::registerTypes();
    Module::registerQmlTypes(uri);

#ifdef FACELIFT_ENABLE_IPC
    // We register the IPC adapters/proxies so that we can use our interfaces across process boundaries
    ModuleIPC::registerQmlTypes(uri);
#endif

    // We are registering the model types here, which can be used by the UI code.
    // The decision to register a dummy, QML, or C++ implementation should be taken here
    registerQmlComponent<AddressBookCppWithProperties>(uri);
    //    registerQmlComponent<AddressBookCpp>(uri);

    ModuleMonitor::registerTypes();

    // Register the dummy implementations
    ModuleDummy::registerQmlTypes(uri);

}
