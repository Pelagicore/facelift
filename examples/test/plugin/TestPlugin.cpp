/*
 *   This file is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the <license name> licence.
 *   Please see the LICENSE file for details.
 */

#include "TestPlugin.h"

#include "facelift/test/Module.h"

#ifdef FACELIFT_ENABLE_IPC
#include "facelift/test/ModuleIPC.h"
#endif

using namespace facelift::test;

void TestPlugin::registerTypes(const char *uri)
{
    // Register the generated types
    Module::registerQmlTypes(uri);

    // We are registering the model types here, which can be used by the UI code.
    // The decision to register a dummy, QML, or C++ implementation should be taken here
    //    registerQmlComponent<testCpp>(uri);

}
