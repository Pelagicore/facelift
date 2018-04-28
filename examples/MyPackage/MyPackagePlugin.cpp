/*
 *   This file is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the <license name> licence.
 *   Please see the LICENSE file for details.
 */

#include "MyPackagePlugin.h"

#include "models/cpp/MyInterfaceCppImplementation.h"
#include "facelift/example/mypackage/Module.h"

#include "facelift/example/mypackage/MyInterfaceQMLImplementation.h"

void MyPackagePlugin::registerTypes(const char *uri)
{
    using namespace facelift::example::mypackage;

    // Register generated QML types
    Module::registerQmlTypes(uri);

    // This additional call registers all interfaces as uncreatable types, so that we can used typed properties in our QML files
    // The type names are exactly the interface names as defined in the QFace file
    Module::registerUncreatableQmlTypes(uri);

    // We register our C++ implementation class as a creatable type, which can be instantiated by the UI code, using the given string identifier
    facelift::registerQmlComponent<MyInterfaceCppImplementation>(uri, "MyInterfaceImplementation");

    // Other possibility: we register our C++ implementation class as a singleton, which can be used by the UI code, using the given string identifier
    //    facelift::registerSingletonQmlComponent<MyInterfaceCppImplementation>(uri, "MyInterfaceSingleton");

}
