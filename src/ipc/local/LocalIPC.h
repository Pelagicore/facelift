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

#include <memory>

#include <QDebug>

#include "FaceliftModel.h"
#include "FaceliftUtils.h"
#include "FaceliftProperty.h"

#include "ipc-common.h"

namespace facelift {

namespace ipc { namespace local {
class ObjectRegistry;
class ObjectRegistryAsync;
} }

namespace local {

using namespace facelift;

class FaceliftIPCLocalLib_EXPORT LocalIPCMessage
{

    enum class MessageType {
        Request,
        Reply,
        Error
    };

public:
    using ReplyFunction = std::function<void(LocalIPCMessage &)>;

    LocalIPCMessage()
    {
    }

    LocalIPCMessage(const char *methodName)
    {
        m_data.m_member = methodName;
    }

    LocalIPCMessage(const LocalIPCMessage &other)
    {
        m_data = other.m_data;
        copyRequestMessage(other);
    }

    LocalIPCMessage &operator=(const LocalIPCMessage &other)
    {
        if (this != &other) {
            m_data = other.m_data;
            copyRequestMessage(other);
        }
        return *this;
    }

    void copyRequestMessage(const LocalIPCMessage &other)
    {
        if (other.m_requestMessage) {
            m_requestMessage = std::make_unique<LocalIPCMessage>(*other.m_requestMessage);
        }
    }

    QString member() const
    {
        return m_data.m_member;
    }

    LocalIPCMessage createReply() const
    {
        LocalIPCMessage reply;
        reply.m_data.m_messageType = MessageType::Reply;
        reply.m_requestMessage = std::make_unique<LocalIPCMessage>(*this);
        return reply;
    }

    LocalIPCMessage createErrorReply()
    {
        auto reply = createReply();
        reply.m_data.m_messageType = MessageType::Error;
        return reply;
    }

    QString toString() const;

    bool isReplyMessage() const
    {
        return (m_data.m_messageType == MessageType::Reply);
    }

    bool isErrorMessage() const
    {
        return (m_data.m_messageType == MessageType::Error);
    }

    OutputPayLoad &outputPayLoad();

    InputPayLoad &inputPayLoad();

    void addListener(const QObject *context, ReplyFunction function)
    {
        m_data.m_listener = function;
        m_data.m_listenerContext = context;
    }

    void notifyListener()
    {
        if (m_requestMessage) {
            if (m_requestMessage->m_data.m_listenerContext) {
                m_requestMessage->m_data.m_listener(*this);
            }
        }
    }

private:
    struct
    {
        QString m_member;
        QByteArray m_payload;
        MessageType m_messageType = MessageType::Request;
        ReplyFunction m_listener;
        QPointer<const QObject> m_listenerContext;
    } m_data;

    std::unique_ptr<OutputPayLoad> m_outputPayload;
    std::unique_ptr<InputPayLoad> m_inputPayload;
    std::unique_ptr<LocalIPCMessage> m_requestMessage;

};


class FaceliftIPCLocalLib_EXPORT LocalIPCRequestHandler
{

public:
    virtual void deserializePropertyValues(LocalIPCMessage &msg, bool isCompleteSnapshot) = 0;
    virtual void deserializeSignal(LocalIPCMessage &msg) = 0;
    virtual void setServiceRegistered(bool isRegistered) = 0;

};


}

}
