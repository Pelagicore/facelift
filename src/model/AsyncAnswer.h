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

#include "TAsyncAnswerMaster.h"
#include "FaceliftCommon.h"

namespace facelift {

template<typename T>
struct CallBackType {
    using type = std::function<void (const T &)>;
};

template<>
struct CallBackType<void> {
    using type = std::function<void ()>;
};

template<typename ReturnType>
class AsyncAnswer
{
    using CallBack = typename CallBackType<ReturnType>::type;

public:
    class Master : public TAsyncAnswerMaster<CallBack>
    {
    public:
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
        if (this != &other) {
            m_master = other.m_master;
        }
        return *this;
    }

    template<typename T = ReturnType>
    typename std::enable_if<!std::is_void<T>::value>::type
    operator()(const T &returnValue) const
    {
        if (m_master) {
            m_master->call(returnValue);
        }
        else {
            TAsyncAnswerMasterBase::onNoCallbackCalled();
        }
    }

    template<typename T = ReturnType>
    typename std::enable_if<std::is_void<T>::value>::type
    operator()() const
    {
        if (m_master) {
            m_master->call();
        } else {
            TAsyncAnswerMasterBase::onNoCallbackCalled();
        }
    }

private:
    std::shared_ptr<Master> m_master;
};

} // namespace facelift

