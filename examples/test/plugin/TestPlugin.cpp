/**********************************************************************
**
** Copyright (C) 2018 Luxoft Sweden AB
**
** This file is part of the FaceLift project
**
** Permission is hereby granted, free of charge, to any person
** obtaining a copy of this software and associated documentation files
** (the "Software"), to deal in the Software without restriction,
** including without limitation the rights to use, copy, modify, merge,
** publish, distribute, sublicense, and/or sell copies of the Software,
** and to permit persons to whom the Software is furnished to do so,
** subject to the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
** BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
** ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
** SPDX-License-Identifier: MIT
**
**********************************************************************/

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
