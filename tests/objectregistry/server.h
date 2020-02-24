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

#ifndef CLIENT_EXECUTABLE_LOCATION
#error "CLIENT_EXECUTABLE_LOCATION must be be defined! Check CMakeLists.txt"
#endif

#include <memory>
#include <QString>
#include "tests/ipc/OtherIPCTestInterfaceImplementationBase.h"
#include "tests/ipc/IPCTestInterfaceImplementationBase.h"
#include "tests/ipc/OtherIPCTestInterfaceIPCAdapter.h"
#include "tests/ipc/IPCTestInterfaceIPCAdapter.h"

namespace tests {
namespace ipc {

class OtherIPCTestInterfaceImpl : public OtherIPCTestInterfaceImplementationBase {
    Q_OBJECT
public:
    OtherIPCTestInterfaceImpl();
    void requestExit() override;

    OtherIPCTestInterfaceIPCAdapter m_adapter;
};

class IPCTestInterfaceImpl : public IPCTestInterfaceImplementationBase {
    Q_OBJECT
public:
    IPCTestInterfaceImpl();
    void registerOtherIPCTestInterface() override;

    IPCTestInterfaceIPCAdapter m_adapter;
    std::unique_ptr<OtherIPCTestInterfaceImpl> m_syncIPCTestInterfaceImpl;
};

} // end namespace ipc
} // end namespace tests
