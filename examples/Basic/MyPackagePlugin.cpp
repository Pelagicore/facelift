/*
 *   This file is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the <license name> licence.
 *   Please see the LICENSE file for details.
 */

#include "MyPackagePlugin.h"

#include "models/cpp/MyInterfaceCppImplementation.h"
#include "mypackage/Module.h"
#include "mypackage/ModuleMonitor.h"
#include "mypackage/ModuleDummy.h"

#ifdef FACELIFT_ENABLE_IPC
#include "mypackage/ModuleIPC.h"
#endif

using namespace mypackage;

void MyPackagePlugin::registerTypes(const char *uri)
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
    facelift::registerQmlComponent<MyInterfaceCppImplementation>(uri);
    //    registerQmlComponent<AddressBookCpp>(uri);

    ModuleMonitor::registerTypes();

    // Register the dummy implementations
    ModuleDummy::registerQmlTypes(uri);

}
