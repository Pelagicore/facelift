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

#include "CombinedTestsPlugin.h"
#include "tests/combined/Module.h"
#include "tests/combined/other/Module.h"
#include "tests/combined/CombinedInterfaceIPCProxy.h"
#include "tests/combined/CombinedInterfaceAsyncIPCProxy.h"
#if defined(QML_IMPL_LOCATION)
#  include "tests/combined/CombinedInterfaceImplementationBaseQML.h"
#else
#  include "impl/cpp/CombinedTestsCppImplementation.h"
#endif


using namespace tests::combined;


void CombinedTestsPlugin::registerTypes(const char *uri)
{
    tests::combined::Module::registerQmlTypes(uri);
    tests::combined::other::Module::registerQmlTypes(uri);

#if defined(QML_IMPL_LOCATION)
    facelift::registerQmlComponent<CombinedInterfaceImplementationBaseQML>(uri, STRINGIFY(QML_IMPL_LOCATION)
            "/impl/qml/CombinedTestsQmlImplementation.qml", "CombinedInterfaceAPI");

    facelift::registerSingletonQmlComponent<CombinedInterfaceImplementationBaseQML>(uri, STRINGIFY(QML_IMPL_LOCATION)
            "/impl/qml/CombinedTestsQmlImplementation.qml", "CombinedInterfaceSingleton");
#else
    facelift::registerQmlComponent<CombinedInterfaceImplementation>(uri, "CombinedInterfaceAPI");

    facelift::registerSingletonQmlComponent<CombinedInterfaceImplementation>(uri, "CombinedInterfaceSingleton");
#endif

    facelift::registerSingletonQmlComponent<CombinedInterfaceIPCProxy>(uri, "CombinedInterfaceIPCProxySingleton");
    facelift::registerSingletonQmlComponent<CombinedInterfaceAsyncIPCProxy>(uri, "CombinedInterfaceAsyncIPCProxySingleton");
}
