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

    // Register all generated QML types
    Module::registerQmlTypes(uri);

    Module::registerUncreatableQmlTypes(uri);

    // We are registering the implementation types here, which can be used by the UI code.
    facelift::registerQmlComponent<MyInterfaceCppImplementation>(uri);
    //facelift::registerQmlComponent<MyInterfaceQMLImplementation>(uri, STRINGIFY(QML_MODEL_LOCATION) "/models/qml/mypackage/MyInterface.qml");
}
