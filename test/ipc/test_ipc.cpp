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

#include "ipc.h"
#include <QDebug>
#include <QApplication>

#include <sys/types.h>
#include <unistd.h>

#include "facelift/test/TestInterfaceDummy.h"
#include "facelift/test/TestInterfaceIPC.h"

using namespace facelift::test;

void mainClient(int &argc, char * *argv)
{
    QApplication app(argc, argv);
    auto sessionBus = QDBusConnection::sessionBus();

    qDebug() << "Client running";

    TestInterfaceIPCProxy proxy;

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&] () {
        qWarning() << "boolProperty" << proxy.boolProperty();
        proxy.method1();
    });
    timer.start(1000);

    QObject::connect(&proxy, &TestInterface::boolPropertyChanged, [&] () {
        qWarning() << "boolProperty changed " << proxy.boolProperty();
    });

    app.exec();
    qDebug() << "Client exited";

}


void mainServer(int &argc, char * *argv)
{
    QApplication app(argc, argv);

    TestStruct2 s;
    s.seti(TestEnum::E3);
    auto byteArray = s.serialize();

    TestStruct2 o;
    o.deserialize(byteArray);

    Q_ASSERT(s == o);

    TestInterfaceDummy testInterface;
    TestInterfaceIPCAdapter svc;
    svc.setService(&testInterface);

    QTimer timer;
    timer.setInterval(1000);
    QObject::connect(&timer, &QTimer::timeout, [&] () {
        //      svc.onPropertyValueChanged();
    });
    timer.start();

    qDebug() << "Server running";
    app.exec();
    qDebug() << "Server exited";

}

int main(int argc, char * *argv)
{

    if (argc == 1) {
        mainServer(argc, argv);
    } else {
        mainClient(argc, argv);
    }

}
