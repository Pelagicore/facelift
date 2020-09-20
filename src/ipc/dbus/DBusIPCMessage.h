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

#if defined(FaceliftIPCLibDBus_LIBRARY)
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_IMPORT
#endif

#include <memory>
#include <QDBusMessage>
#include <QByteArray>

namespace facelift {

class OutputPayLoad;
class InputPayLoad;

namespace dbus {

class FaceliftIPCLibDBus_EXPORT DBusIPCMessage
{

public:
    DBusIPCMessage();
    DBusIPCMessage(const DBusIPCMessage &other);
    DBusIPCMessage &operator=(const DBusIPCMessage &other);
    DBusIPCMessage(const QDBusMessage &msg);
    DBusIPCMessage(const QString &service, const QString &path, const QString &interface, const QString &method);
    DBusIPCMessage(const QString &path, const QString &interface, const QString &signal);

    QString member() const;
    QString toString() const;
    QList<QVariant> arguments() const;
    DBusIPCMessage &operator<<(const QVariant &arg);
    DBusIPCMessage createReply();
    DBusIPCMessage createErrorReply(const QString &name, const QString &msg);
    QString signature() const;
    bool isReplyMessage() const;
    bool isErrorMessage() const;
    QDBusMessage& outputMessage();

private:
    QDBusMessage m_message;
};

} // end namespace dbus
} // end namespace facelift
