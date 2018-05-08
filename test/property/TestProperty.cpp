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

#define EXPECT_TRUE(cond) if (!(cond)) { qFatal("Expectation wrong:" #cond "%s", qPrintable(__PRETTY_FUNCTION__)); };

int main(int argc, char *argv[])
{
    Q_UNUSED(argc)
    Q_UNUSED(argv)

    TestPropertyClass c;
    facelift::Property<int> intProperty = 7;
    bool signalTriggered = false;
    QObject::connect(&c, &TestPropertyClass::aSignal, [&] () {
        signalTriggered = true;
    });
    intProperty.init(&c, &TestPropertyClass::aSignal);

    intProperty = 9;  // We expect the signal to be triggered here
    EXPECT_TRUE(signalTriggered);

    return 0;
}
