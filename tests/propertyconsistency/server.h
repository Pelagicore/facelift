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
#include "tests/ipc/IPCConsistencyTestInterfaceIPCAdapter.h"

namespace tests {
namespace ipc {

/**
 * This implementation exposes 2 properties which always contain identical values
 * The IPC framework ensures that client side proxy objects also provide the same values
 */
class IPCTestInterfaceImpl : public IPCConsistencyTestInterface {
    Q_OBJECT
public:
    IPCTestInterfaceImpl();

    void toggle() override;

    const bool& boolProperty1() const override {
        return m_boolProperty;
    }

    const bool& boolProperty2() const override {
        return m_boolProperty;
    }

    bool ready() const override {
        return true;
    }

    IPCConsistencyTestInterfaceIPCAdapter m_adapter;
    QTimer m_timer;
    bool m_boolProperty = false;
};

} // end namespace ipc
} // end namespace tests
