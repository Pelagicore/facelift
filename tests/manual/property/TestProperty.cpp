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

#include <QtCore>
#include <QDebug>

#include "FaceliftProperty.h"
#include "TestProperty.h"

#include "facelift-test.h"


static void testInteger()
{
    TestPropertyClass c;
    facelift::Property<int> prop = 7;
    SignalSpy signalSpy(&c, &TestPropertyClass::aSignal);
    prop.init(&c, &TestPropertyClass::aSignal);

    prop = 7;  // We expect no signal to be triggered here
    EXPECT_TRUE(!signalSpy.wasTriggered());

    prop = 8;  // We expect the signal to be triggered here
    EXPECT_TRUE(signalSpy.wasTriggered());
}


static void testService()
{
    TestPropertyClass c;
    facelift::ServiceProperty<QObject> prop;
    SignalSpy signalSpy(&c, &TestPropertyClass::aSignal);
    prop.init(&c, &TestPropertyClass::aSignal);

    signalSpy.reset();
    prop = nullptr;  // We expect no signal to be triggered here
    EXPECT_TRUE(!signalSpy.wasTriggered());

    signalSpy.reset();
    QObject o;
    prop = &o;  // We expect the signal to be triggered here
    EXPECT_TRUE(signalSpy.wasTriggered());

    signalSpy.reset();
    prop = &o;  // We expect no signal to be triggered here
    EXPECT_TRUE(!signalSpy.wasTriggered());

    signalSpy.reset();
    auto o2 = new QObject();
    prop = o2;  // We expect a signal to be triggered here
    EXPECT_TRUE(signalSpy.wasTriggered());

    // Assign the same object another time => no signal
    signalSpy.reset();
    prop = o2;  // We expect no signal to be triggered here
    EXPECT_TRUE(!signalSpy.wasTriggered());

    // Delete the previously assigned object and allocate a new one (which is going to have the same address as the previous one) => signal
    signalSpy.reset();
    qCWarning(LogGeneral) << o2;
    delete o2;
    o2 = new QObject();
    qCWarning(LogGeneral) << o2;
    prop = o2;  // We expect a signal to be triggered here
    EXPECT_TRUE(signalSpy.wasTriggered());

    signalSpy.reset();
    qCWarning(LogGeneral) << o2;
    o2 = new QObject();
    qCWarning(LogGeneral) << o2;
    prop = o2;  // We expect a signal to be triggered here
    EXPECT_TRUE(signalSpy.wasTriggered());

}

int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    testInteger();
    testService();

    return 0;
}
