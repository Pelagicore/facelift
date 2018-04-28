/*
 *   This file is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the <license name> licence.
 *   Please see the LICENSE file for details.
 */

#include "TestPlugin.h"
#include "facelift/test/Module.h"
#include "models/cpp/TestInterfaceCppImplementation.h"
#include "facelift/test/TestInterfaceQMLImplementation.h"

#ifdef FACELIFT_ENABLE_IPC
#  include "facelift/test/ModuleIPC.h"
#endif

using namespace facelift::test;

void TestPlugin::registerTypes(const char *uri)
{
    // Register the generated types
    Module::registerQmlTypes(uri);

    // This additional call registers all interfaces as uncreatable types, so that we can use typed properties in our
    // QML files. The type names are exactly the interface names as defined in the QFace file
    Module::registerUncreatableQmlTypes(uri);

#if !defined(QML_MODEL_LOCATION)
    // This will register the "TestInterfaceQMLFrontend" class as an instantiatable QML type, named
    // "TestInterfaceImplementation". It's actual implementation will be delegated to "TestInterfaceCppImplementation".
    facelift::registerQmlComponent<TestInterfaceCppImplementation>(uri, "TestInterfaceImplementation");
#else
    // This will register the "TestInterfaceQMLFrontend" class as an instantiatable QML type, named
    // "TestInterfaceImplementation". It's actual implementation will be delegated to the "TestInterface" QML
    // component, that derives from "TestInterfaceQMLImplementation".
    facelift::registerQmlComponent<TestInterfaceQMLImplementation>(uri, STRINGIFY(QML_MODEL_LOCATION)
            "/models/qml/TestInterface.qml", "TestInterfaceImplementation");
#endif
}
