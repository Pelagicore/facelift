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

#include "facelift-ipc.h"
#include "facelift-test.h"

#include <QCoreApplication>

#include "TestInterfaceCppImplementation.h"
#include "facelift/test/TestInterfaceIPCProxy.h"
#include "facelift/test/TestInterfaceIPCAdapter.h"

using namespace facelift::test;


void checkInterface(TestInterface &i)
{
    EXPECT_TRUE(i.ready());

    EXPECT_TRUE(i.interfaceProperty() != nullptr);

    SignalSpy signalSpy(&i, &TestInterface::aSignal);
    i.interfaceProperty()->triggerMainInterfaceSignal(100);
    EXPECT_TRUE(signalSpy.wasTriggered());

}


int main(int argc, char * *argv)
{
    QCoreApplication app(argc, argv);

    TestInterfaceCppImplementation i;

    checkInterface(i);

    TestInterfaceIPCAdapter ipcAdapter;
    ipcAdapter.setService(&i);
    ipcAdapter.init();

    TestInterfaceIPCProxy proxy;
    proxy.connectToServer();
    checkInterface(proxy);

    //    TestInterfaceIPCProxyNew proxy2;
    //    proxy2.connectToServer();
    //    checkInterface(proxy2);

}
