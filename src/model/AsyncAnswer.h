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

#include <memory>
#include <functional>

#include <QObject>
#include <QDebug>
#include <QPointer>

#include "FaceliftCommon.h"

#if defined(FaceliftModelLib_LIBRARY)
#  define FaceliftModelLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftModelLib_EXPORT Q_DECL_IMPORT
#endif


namespace facelift {

class TAsyncAnswerMasterBase {

public:

    TAsyncAnswerMasterBase(QObject* context)
    {
        m_context = context;
    }

    ~TAsyncAnswerMasterBase();

protected:

    void setAnswered()
    {
        Q_ASSERT(m_isAlreadyAnswered == false);
        m_isAlreadyAnswered = true;
    }

    bool m_isAlreadyAnswered = false;
    QPointer<QObject> m_context;

};

template<typename CallBack>
class TAsyncAnswerMaster : private TAsyncAnswerMasterBase
{

public:
    TAsyncAnswerMaster(QObject* context, CallBack callback) : TAsyncAnswerMasterBase(context), m_callback(callback)
    {
    }

    template<typename ... Types>
    void call(const Types & ... args)
    {
        setAnswered();
        if (m_context)
        {
            qWarning() << "Calling callback with context" << m_context.data();;
            m_callback(args ...);
        }
        else
            qCritical() << "Context destroyed";
    }

protected:
    CallBack m_callback;
};


template<typename ReturnType>
class AsyncAnswer
{
    typedef std::function<void (const ReturnType &)> CallBack;

public:
    class Master : public TAsyncAnswerMaster<CallBack>
    {
    public:
        using TAsyncAnswerMaster<CallBack>::m_callback;
        Master(QObject* context, CallBack callback) : TAsyncAnswerMaster<CallBack>(context, callback)
        {
        }
    };

    AsyncAnswer()
    {
    }

    AsyncAnswer(QObject* context, CallBack callback) : m_master(new Master(context, callback))
    {
    }

    AsyncAnswer(const AsyncAnswer &other) : m_master(other.m_master)
    {
    }

    AsyncAnswer &operator=(const AsyncAnswer &other)
    {
        m_master = other.m_master;
        return *this;
    }

    void operator()(const ReturnType &returnValue) const
    {
        if (m_master) {
            qWarning() << "Calling callback";
            m_master->call(returnValue);
            qWarning() << "callback called";
        }
        else {
            qCritical() << "No master";
        }
    }

private:
    std::shared_ptr<Master> m_master;
};

template<>
class AsyncAnswer<void>
{
    typedef std::function<void ()> CallBack;

public:
    class Master : public TAsyncAnswerMaster<CallBack>
    {
    public:
        using TAsyncAnswerMaster<CallBack>::m_callback;

        Master(QObject* context, CallBack callback) : TAsyncAnswerMaster<CallBack>(context, callback)
        {
        }
    };

    AsyncAnswer()
    {
    }

    AsyncAnswer(QObject* context, CallBack callback) : m_master(new Master(context, callback))
    {
    }

    AsyncAnswer(const AsyncAnswer &other) : m_master(other.m_master)
    {
    }

    void operator()() const
    {
        if (m_master) {
            m_master->call();
        }
    }

private:
    std::shared_ptr<Master> m_master;
};

}

