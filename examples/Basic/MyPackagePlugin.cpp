/*
 *   This file is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the <license name> licence.
 *   Please see the LICENSE file for details.
 */

#include "MyPackagePlugin.h"

#include "facelift/example/mypackage/Module.h"

#include "models/cpp/MyInterfaceCppImplementation.h"
#include "facelift/example/mypackage/MyInterfaceQMLImplementation.h"

void MyPackagePlugin::registerTypes(const char *uri)
{
    using namespace facelift::example::mypackage;

    // Register the generated types
    Module::registerQmlTypes(uri);

    // We are registering the model types here, which can be used by the UI code.
    // The decision to register a dummy, QML, or C++ implementation should be taken here
    //    facelift::registerQmlComponent<MyInterfaceCppImplementation>(uri);
    facelift::registerQmlComponent<MyInterfaceQMLImplementation>(uri, STRINGIFY(QML_MODEL_LOCATION) "/models/qml/mypackage/MyInterface.qml");
}
