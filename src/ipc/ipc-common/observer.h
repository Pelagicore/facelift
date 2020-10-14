/**********************************************************************
**
** Copyright (C) 2020 Luxoft Sweden AB
**
** This file is part of the FaceLift project
**
** Permission is hereby granted, freIPCServiceAdapterBasee of charge, to any person
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

#include <memory>
#include <QObject>

namespace facelift {

class IObserver : public QObject
{
    Q_OBJECT
public:
    virtual void onReadyChanged(std::shared_ptr<QMetaObject::Connection> connection ) = 0;
};

// Single-time observer which will unregister itself when done
template<typename Function>
class SingleTimeObserver : public IObserver
{
    Function m_func;
public:
    explicit SingleTimeObserver(Function func): m_func{func} {}
    ~SingleTimeObserver()=default;

    void onReadyChanged(std::shared_ptr<QMetaObject::Connection> connection) override {
         m_func();
         QObject::disconnect(*connection);
    }
};
// Standard observer which will work for each signal
template<typename Function>
class StandartObserver : public IObserver
{
    Function m_func;
public:
    explicit StandartObserver(Function func): m_func{func} {}
    ~StandartObserver()=default;

    void onReadyChanged(std::shared_ptr<QMetaObject::Connection> connection ) override {
        Q_UNUSED(connection)
        m_func();
    }
};
}
