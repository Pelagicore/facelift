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

#include <QTextStream>
#include "DBusIPCMessage.h"
#include "ipc-common.h"

namespace facelift {
namespace dbus {

DBusIPCMessage::DBusIPCMessage() :
    DBusIPCMessage("dummy/dummy", "dummy.dummy", "gg")
{
}

DBusIPCMessage::DBusIPCMessage(const DBusIPCMessage &other) : m_message(other.m_message)
{
}

DBusIPCMessage &DBusIPCMessage::operator=(const DBusIPCMessage &other)
{
    if (this != &other) {
        m_message = other.m_message;
    }
    return *this;
}

DBusIPCMessage::DBusIPCMessage(const QDBusMessage &msg)
{
    m_message = msg;
}

DBusIPCMessage::DBusIPCMessage(const QString &service, const QString &path, const QString &interface, const QString &method)
{
    m_message = QDBusMessage::createMethodCall(service, path, interface, method);
}

DBusIPCMessage::DBusIPCMessage(const QString &path, const QString &interface, const QString &signal)
{
    m_message = QDBusMessage::createSignal(path, interface, signal);
}

QList<QVariant> DBusIPCMessage::arguments() const
{
    return m_message.arguments();
}

DBusIPCMessage &DBusIPCMessage::operator<<(const QVariant &arg)
{
    static int qListStringTypeId = qMetaTypeId<QList<QString>>();
    if (arg.userType() != qListStringTypeId) { // workaround to use QList<QString> since its signature matches the QStringList
        m_message << arg;
    }
    else {
        QStringList stringList(arg.value<QList<QString>>());
        m_message << stringList;
    }
    return *this;
}

QString DBusIPCMessage::member() const
{
    return m_message.member();
}

QString DBusIPCMessage::toString() const
{
    QString str;
    QTextStream s(&str);

    s << "DBus message ";
    s << " service:" << m_message.service();
    s << " interface:" << m_message.interface();
    s << " path:" << m_message.path();
    s << " member:" << m_message.member();

    s << " / Arguments : [ ";
    for (auto &arg : m_message.arguments()) {
        s << arg.toString() << ", ";
    }
    s << " ]";

    s << " signature:" << m_message.signature();

    return str;
}

DBusIPCMessage DBusIPCMessage::createReply()
{
    return DBusIPCMessage(m_message.createReply());
}

DBusIPCMessage DBusIPCMessage::createErrorReply(const QString &msg, const QString &member)
{
    return DBusIPCMessage(m_message.createErrorReply(msg, member));
}

QString DBusIPCMessage::signature() const
{
    return m_message.signature();
}

bool DBusIPCMessage::isReplyMessage() const
{
    return (m_message.type() == QDBusMessage::ReplyMessage);
}

bool DBusIPCMessage::isErrorMessage() const
{
    return (m_message.type() == QDBusMessage::ErrorMessage);
}

QDBusMessage& DBusIPCMessage::outputMessage() {
    return m_message;
}

} // end namespace dbus
} // end namespace facelift
