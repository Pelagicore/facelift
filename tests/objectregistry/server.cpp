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
#include "server.h"
#include <QCoreApplication>
#include <QProcess>

namespace tests {
namespace ipc {

OtherIPCTestInterfaceImpl::OtherIPCTestInterfaceImpl()
{
    m_adapter.registerService(this);
}

void OtherIPCTestInterfaceImpl::requestExit()
{
    emit exitRequested();
    qDebug() << "Test passed";
    QCoreApplication::quit();
}

IPCTestInterfaceImpl::IPCTestInterfaceImpl()
{
    m_adapter.registerService(this);
}

void IPCTestInterfaceImpl::registerOtherIPCTestInterface()
{
    qDebug() << "registering OtherIPCTestInterface adapter";
    m_syncIPCTestInterfaceImpl = std::make_unique<OtherIPCTestInterfaceImpl>();
    QObject::connect(m_syncIPCTestInterfaceImpl.get(), &OtherIPCTestInterfaceImpl::exitRequested, this, &IPCTestInterfaceImpl::exitRequested);
}

} // end namespace ipc
} // end namespace tests

int main(int argc, char** argv) {

    QCoreApplication app(argc, argv);

    tests::ipc::IPCTestInterfaceImpl server;
    QProcess client;

    // timeout in case something goes wrong
    QTimer::singleShot(10000, &app, [&app, &client](){
        qDebug() << "Test failed: timeout";
        client.close();
        app.exit(1);
    });

    QObject::connect(&server, &tests::ipc::IPCTestInterfaceImpl::exitRequested, &client, &QProcess::close);
    client.start(CLIENT_EXECUTABLE_LOCATION);

    return app.exec();
}
