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

//! [indoc]
#pragma once

#include "facelift/example/mypackage/MyInterfacePropertyAdapter.h"


using namespace facelift::example::mypackage;

/**
 * C++ Implementation of the MyInterface API
 */
class MyInterfaceCppImplementation : public MyInterfacePropertyAdapter
{

public:
    MyInterfaceCppImplementation(QObject *parent = nullptr) : MyInterfacePropertyAdapter(parent)
    {
        connect(&m_timer, &QTimer::timeout, this, [this] () {
            m_counter++;   // The value change signal is automatically triggered for you here
        });
        m_timer.start(3000);
    }

    void resetCounter(int delay) override
    {
        QTimer::singleShot(delay, [this]() mutable {
            m_counter = 0;   // This assignment triggers the corresponding "value changed" signal.
            counterReset();
        });
    }

    void resetCounterAsync(int delay, facelift::AsyncAnswer<void> answer) override
    {
        QTimer::singleShot(1000, this, [this, answer, delay] () {
            resetCounter(delay);
            answer();
        });
    }

    static MyInterfaceCppImplementation &instance()
    {
        static MyInterfaceCppImplementation i;
        return i;
    }

private:
    QTimer m_timer;

};
//! [indoc]
