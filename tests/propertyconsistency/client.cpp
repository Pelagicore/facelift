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

void tests::ipc::Tester::start()
{
    m_async = std::make_unique<tests::ipc::IPCConsistencyTestInterfaceAsyncIPCProxy>();

    auto checkConsistency = [this]() {
        if (m_async->boolProperty1() != m_async->boolProperty2()) {
            qCritical() << "Inconsistency detected";
            qApp->exit(10);
        }
    };

    connect(m_async.get(), &facelift::InterfaceBase::readyChanged, this, [this, checkConsistency]() {
        if (m_async->ready()) {
            checkConsistency();
            m_async->toggle();
        }
    });

    connect(m_async.get(), &IPCConsistencyTestInterfaceAsyncIPCProxy::boolProperty1Changed, this, checkConsistency);
    connect(m_async.get(), &IPCConsistencyTestInterfaceAsyncIPCProxy::boolProperty2Changed, this, checkConsistency);

    QTimer::singleShot(3000, [&]() {
        if (!m_async->ready()) {
            qCritical() << "Server not found. path:" << m_async->objectPath();
            qApp->exit(2);
        } else {
            qApp->exit(0);
        }
    });

    m_async->connectToServer();
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    tests::ipc::Tester tester;
    tester.start();

    return app.exec();
}
