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
#pragma once

#include <memory>
#include <QObject>
#include "tests/ipc/IPCTestInterfaceAsyncIPCProxy.h"
#include "tests/ipc/OtherIPCTestInterfaceIPCProxy.h"

namespace tests {
namespace ipc {

class Tester : public QObject {
    Q_OBJECT
public:
    /**
     * @brief runTest: tests getting objects in the blocking way, before the non-blocking
     * call result is received
     *
     * scenario:
     * 1. connect async proxy to DBus (this will trigger getObjects async call)
     * 2. trigger registering another proxy on the server side
     * 3. create and connect sync proxy to DBus  (this will trigger getObjects sync call)
     * 4. call a mathod on sync proxy. Successful call ends test.
     */
    void runTest();

    std::unique_ptr<IPCTestInterfaceAsyncIPCProxy> m_async;
    std::unique_ptr<OtherIPCTestInterfaceIPCProxy> m_sync;
    void registerAnotherAdapter();
};

} // end namespace ipc
} // end namespace tests
