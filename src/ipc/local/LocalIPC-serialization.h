/**********************************************************************
**
** Copyright (C) 2019 Luxoft Sweden AB
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

#if defined(FaceliftIPCLocalLib_LIBRARY)
#  define FaceliftIPCLocalLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCLocalLib_EXPORT Q_DECL_IMPORT
#endif

#include "LocalIPCMessage.h"
#include "LocalIPCProxy.h"
#include "LocalIPCServiceAdapter.h"
#include "FaceliftIPCCommon.h"

namespace facelift {

namespace local {

template<typename ... Args>
inline LocalIPCMessage LocalIPCProxyBinder::sendMethodCall(const char *methodName, Args && ... args) const
{
    LocalIPCMessage msg(methodName);
    using expander = int[];
        (void)expander{0,
            (void(msg << QVariant::fromValue(std::forward<Args>(args))), 0)...
        };
    auto replyMessage = call(msg);
    if (replyMessage.isErrorMessage()) {
        onServerNotAvailableError(methodName);
    }
    return replyMessage;
}

template<typename ReturnType, typename ... Args>
inline void LocalIPCProxyBinder::sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<ReturnType> answer, Args && ... args)
{
    LocalIPCMessage msg(methodName);
    using expander = int[];
        (void)expander{0,
            (void(msg << QVariant::fromValue(std::forward<Args>(args))), 0)...
        };
    asyncCall(msg, this, [this, answer](LocalIPCMessage &msg) {
                ReturnType returnValue;
                if (msg.isReplyMessage()) {
                    returnValue = (!msg.arguments().isEmpty() ? qvariant_cast<ReturnType>(msg.arguments().first()): ReturnType());
                    answer(returnValue);
                } else {
                    qCWarning(LogIpc) << "Error received" << msg.toString();
                }
            });
}

template<typename ... Args>
inline void LocalIPCProxyBinder::sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<void> answer, Args && ... args)
{
    LocalIPCMessage msg(methodName);
    using expander = int[];
        (void)expander{0,
            (void(msg << QVariant::fromValue(std::forward<Args>(args))), 0)...
        };
    asyncCall(msg, this, [answer](LocalIPCMessage &msg) {
                Q_UNUSED(msg);
                answer();
            });
}

template<typename PropertyType>
inline void LocalIPCProxyBinder::sendSetterCall(const QString& property, const PropertyType &value)
{
    LocalIPCMessage msg(FaceliftIPCCommon::PROPERTIES_INTERFACE_NAME, FaceliftIPCCommon::SET_PROPERTY_MESSAGE_NAME);
    msg << QVariant::fromValue(m_interfaceName);
    msg << QVariant::fromValue(property);
    msg << QVariant::fromValue(QVariant::fromValue(value));
    if (isSynchronous()) {
        auto replyMessage = call(msg);
        if (replyMessage.isErrorMessage()) {
            onServerNotAvailableError(property);
        }
    } else {
        asyncCall(msg, this, [this, property](const LocalIPCMessage &replyMessage) {
                    if (replyMessage.isErrorMessage()) {
                        onServerNotAvailableError(property);
                    }
                });
    }
}


template<typename ... Args>
inline void LocalIPCServiceAdapterBase::sendSignal(const QString& signalName, Args && ... args)
{
    LocalIPCMessage signal(signalName);
    using expander = int[];
        (void)expander{0,
            (void(signal << QVariant::fromValue(std::forward<Args>(args))), 0)...
        };

    this->send(signal);
}

template<typename ReturnType>
inline void LocalIPCServiceAdapterBase::sendAsyncCallAnswer(LocalIPCMessage &replyMessage, const ReturnType returnValue)
{
    replyMessage << QVariant::fromValue(returnValue);
    sendReply(replyMessage);
}

}

}
