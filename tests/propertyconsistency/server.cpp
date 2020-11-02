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

IPCTestInterfaceImpl::IPCTestInterfaceImpl()
{
    m_adapter.registerService(this);

    connect(&m_timer, &QTimer::timeout, this, &IPCTestInterfaceImpl::toggle);
    m_timer.start(100);
}

void IPCTestInterfaceImpl::toggle()
{
    m_boolProperty = !m_boolProperty;
    boolProperty1Changed();
    boolProperty2Changed();
}

} // end namespace ipc
} // end namespace tests

int main(int argc, char** argv) {

    QCoreApplication app(argc, argv);

    tests::ipc::IPCTestInterfaceImpl server;
    QProcess client;

    auto exitWithCode = [&app](int code) {
        qCritical() << "Exiting with code" << code;
        app.exit(code);
    };

    // Terminate with error after 10 seconds
    QTimer::singleShot(10000, &app, [&]() {
        qDebug() << "Test failed: timeout";
        client.close();
        exitWithCode(1);
    });

    // We terminate if the client process terminates
    QObject::connect(&client, &QProcess::stateChanged, [&] (QProcess::ProcessState state) {
        qWarning() << "Client process state" << state;
        if (state == QProcess::ProcessState::NotRunning) {
            qWarning() << "Client terminated with status" << client.exitStatus() << client.exitCode();
            if (client.exitStatus() != QProcess::ExitStatus::NormalExit)
                exitWithCode(1);
            else {
                exitWithCode(client.exitCode());
            }

        }
    });

    client.start(CLIENT_EXECUTABLE_LOCATION);

    return app.exec();
}
