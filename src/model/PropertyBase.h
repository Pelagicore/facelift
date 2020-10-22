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

#include <QString>
#include <QObject>

#include "FaceliftCommon.h"

namespace facelift {

class PropertyBase
{

public:
    typedef void (QObject::*ChangeSignal)();

    PropertyBase();
    virtual ~PropertyBase();
    PropertyBase(PropertyBase const &) = delete;
    PropertyBase(PropertyBase const &&) = delete;
    PropertyBase& operator=(PropertyBase const &x) = delete;
    PropertyBase& operator=(PropertyBase const &&x) = delete;

    template<typename ServiceType>
    void init(QObject *ownerObject, void (ServiceType::*changeSignal)(), const char *name = "Unknown")
    {
        m_ownerObject = ownerObject;
        m_ownerSignal = static_cast<ChangeSignal>(changeSignal);
        m_name = name;
    }

    template<typename ServiceType>
    void setReadyChangedSlot(void (ServiceType::*readySignal)())
    {
        m_readySignal = static_cast<ChangeSignal>(readySignal);
    }

    void triggerValueChangedSignal();

    QObject *owner() const
    {
        return m_ownerObject;
    }

    const char *name() const
    {
        return m_name;
    }

    ChangeSignal signalPointer() const
    {
        return m_ownerSignal;
    }

    bool &isReady()
    {
        return m_ready;
    }

    void setReady(bool ready);

protected:
    virtual void clean() = 0;

    virtual bool isDirty() const = 0;

    virtual QString toString() const = 0;

    void doBreakBinding();

private:
    void doTriggerChangeSignal();

    QObject *m_ownerObject = nullptr;
    ChangeSignal m_ownerSignal = nullptr;
    ChangeSignal m_readySignal = nullptr;

    const char *m_name = nullptr;
    bool m_notificationTimerEnabled = false;
    bool m_asynchronousNotification = false;
    bool m_ready = true;

protected:
    QMetaObject::Connection m_getterFunctionContextConnection;
    QVector<QMetaObject::Connection> m_connections;  /// The list of connections which this property is bound to

};

}

