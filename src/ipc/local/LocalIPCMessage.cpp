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

#include <QObject>
#include <QTextStream>

#include "LocalIPCMessage.h"
#include "LocalIPC-serialization.h"

namespace facelift {
namespace local {


QString LocalIPCMessage::toString() const
{
    QString str;
    QTextStream s(&str);

    s << "Local IPC message ";
    s << " member:" << m_data.m_member;
    s << m_data.m_arguments;
    return str;
}

LocalIPCMessage::LocalIPCMessage()
{
}

LocalIPCMessage::LocalIPCMessage(const QString &methodName)
{
    m_data.m_member = methodName;
}

LocalIPCMessage::LocalIPCMessage(const QString &interface, const char *methodName)
{
    m_data.m_interface = interface;
    m_data.m_member = methodName;
}

LocalIPCMessage::LocalIPCMessage(const LocalIPCMessage &other)
{
    m_data = other.m_data;
    copyRequestMessage(other);
}

LocalIPCMessage &LocalIPCMessage::operator=(const LocalIPCMessage &other)
{
    if (this != &other) {
        m_data = other.m_data;
        copyRequestMessage(other);
    }
    return *this;
}

void LocalIPCMessage::copyRequestMessage(const LocalIPCMessage &other)
{
    if (other.m_requestMessage) {
        m_requestMessage = std::make_unique<LocalIPCMessage>(*other.m_requestMessage);
    }
}

QList<QVariant> LocalIPCMessage::arguments() const
{
    return m_data.m_arguments;
}

LocalIPCMessage &LocalIPCMessage::operator<<(const QVariant &arg)
{
    m_data.m_arguments.append(arg);
    return *this;
}

LocalIPCMessage LocalIPCMessage::createReply() const
{
    LocalIPCMessage reply;
    reply.m_data.m_messageType = MessageType::Reply;
    reply.m_requestMessage = std::make_unique<LocalIPCMessage>(*this);
    return reply;
}

 LocalIPCMessage LocalIPCMessage::createErrorReply()
{
    auto reply = createReply();
    reply.m_data.m_messageType = MessageType::Error;
    return reply;
}

void LocalIPCMessage::addListener(const QObject *context, ReplyFunction function)
{
    m_data.m_listener = function;
    m_data.m_listenerContext = context;
}

void LocalIPCMessage::notifyListener()
{
    if (m_requestMessage) {
        if (m_requestMessage->m_data.m_listenerContext) {
            m_requestMessage->m_data.m_listener(*this);
        }
    }
}

}

}
