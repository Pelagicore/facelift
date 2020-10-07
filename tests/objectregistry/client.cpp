/**********************************************************************
**
** Copyright (C) 2020 Luxoft Sweden AB
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
#include "client.h"
#include <QCoreApplication>
#include "DBusManager.h"
#include "DBusIPCCommon.h"
#include "DBusIPCMessage.h"
#include <QDBusPendingCall>

void tests::ipc::Tester::registerAnotherAdapter()
{
    facelift::dbus::DBusIPCMessage msg(
        facelift::dbus::DBusIPCCommon::DEFAULT_SERVICE_NAME,
        m_async->ipc()->objectPath(),
        tests::ipc::IPCTestInterfaceAsyncIPCProxy::FULLY_QUALIFIED_INTERFACE_NAME,
        "registerOtherIPCTestInterface");
    QDBusConnection::sessionBus().call(msg.outputMessage());
}

void tests::ipc::Tester::runTest()
{
    m_async = std::make_unique<tests::ipc::IPCTestInterfaceAsyncIPCProxy>();

    // connectToServer triggers async getObjects, result will be there
    // as soon as the control gets back to event loop
    m_async->connectToServer();

    // force SyncIPCTestInterfaceIPCProxy adapter registration on the server side
    registerAnotherAdapter();

    m_sync = std::make_unique<tests::ipc::OtherIPCTestInterfaceIPCProxy>();
    m_sync->connectToServer();

    // this call will work only if everything works fine. It will also end the test.
    m_sync->requestExit();
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    tests::ipc::Tester tester;
    tester.runTest();

    return app.exec();
}
