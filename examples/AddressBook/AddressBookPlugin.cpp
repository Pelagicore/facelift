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

#include "AddressBookPlugin.h"

#include "facelift/example/addressbook/Module.h"

#include "models/cpp/advanced/AddressBookCppWithProperties.h"
#include "models/cpp/AddressBookCpp.h"

#include "facelift/example/addressbook/AddressBookImplementationBaseQML.h"

using namespace facelift::example::addressbook;

void AddressBookPlugin::registerTypes(const char *uri)
{
    // Register the generated types
    Module::registerQmlTypes(uri);

    facelift::registerUncreatableQmlComponent<AddressBookCppWithProperties>(uri, "AddressBookUncreateAble");

    // We are registering the model types here, which can be used by the UI code.
    // The decision to register a dummy, QML, or C++ implementation should be taken here
    facelift::registerQmlComponent<AddressBookCppWithProperties>(uri, "AddressBook");
    // facelift::registerQmlComponent<AddressBookCpp>(uri, , "AddressBookImpl");
    // facelift::registerQmlComponent<AddressBookImplementationBaseQML>(uri, STRINGIFY(QML_MODEL_LOCATION) "/models/qml/addressbook/AddressBook.qml");
}
