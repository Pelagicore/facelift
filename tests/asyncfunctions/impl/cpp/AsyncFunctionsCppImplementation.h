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

#pragma once

#include "tests/asyncfunctions/AsyncFunctionsInterfaceImplementationBase.h"

using namespace tests::asyncfunctions;

class AsyncFunctionsInterfaceImplementation : public AsyncFunctionsInterfaceImplementationBase
{
public:

    AsyncFunctionsInterfaceImplementation(QObject *parent = nullptr) : AsyncFunctionsInterfaceImplementationBase(parent)
    {
    }

    void getIntValueAsync(facelift::AsyncAnswer<int> answer) override
    {
        QTimer::singleShot(1, [answer]() mutable {
            int returnValue = 10;
            answer(returnValue);
        });
    }

    void getStringValueAsync(facelift::AsyncAnswer<QString> answer) override
    {
        QTimer::singleShot(1, [answer]() mutable {
            answer(QString("Test-String"));
        });
    }

    void getContainerValueAsync(facelift::AsyncAnswer<Container> answer) override
    {
        QTimer::singleShot(1, [answer]() mutable {
            Container c;
            c.setintegerData(9);
            c.setstringData(QString("Test-String"));
            QMap<QString, int> m;
            m["Test-String"] = 27;
            c.setmapData(m);
            answer(c);
        });
    }

    void calculateSumAsync(int arg1, int arg2, int arg3, facelift::AsyncAnswer<int> answer) override
    {
        QTimer::singleShot(1, [answer, arg1, arg2, arg3]() mutable {
            answer(arg1 + arg2 + arg3);
        });
    }

    void append(const QString &str1, const QString &str2, facelift::AsyncAnswer<QString> answer) override
    {
        QTimer::singleShot(1, [answer, str1, str2]() mutable {
            QString sum =  str1 + str2;
            answer(sum);
        });
    }
};
