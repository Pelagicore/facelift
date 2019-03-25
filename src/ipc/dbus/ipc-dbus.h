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

#if defined(FaceliftIPCLibDBus_LIBRARY)
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_IMPORT
#endif

#include <memory>

#include <QDebug>

#include <QDBusConnection>
#include <QDBusMessage>

#include "FaceliftModel.h"
#include "FaceliftUtils.h"
#include "FaceliftProperty.h"

#include "ipc-common/ipc-common.h"
#include "FaceliftLogging.h"

namespace facelift {

namespace ipc { namespace dbus {
class ObjectRegistry;
class ObjectRegistryAsync;
} }

namespace dbus {

using namespace facelift;

class OutputPayLoad;
class InputPayLoad;


class FaceliftIPCLibDBus_EXPORT DBusIPCMessage
{

public:
    DBusIPCMessage() :
        DBusIPCMessage("dummy/dummy", "dummy.dummy", "gg")
    {
    }

    DBusIPCMessage(const DBusIPCMessage &other) : m_message(other.m_message)
    {
    }

    DBusIPCMessage &operator=(const DBusIPCMessage &other)
    {
        m_message = other.m_message;
        return *this;
    }

    DBusIPCMessage(const QDBusMessage &msg)
    {
        m_message = msg;
    }

    DBusIPCMessage(const QString &service, const QString &path, const QString &interface, const QString &method)
    {
        m_message = QDBusMessage::createMethodCall(service, path, interface, method);
    }

    DBusIPCMessage(const QString &path, const QString &interface, const QString &signal)
    {
        m_message = QDBusMessage::createSignal(path, interface, signal);
    }

    DBusIPCMessage call(const QDBusConnection &connection);

    void asyncCall(const QDBusConnection &connection, const QObject *context, std::function<void(DBusIPCMessage &message)> callback);

    void send(const QDBusConnection &connection);

    QString member() const
    {
        return m_message.member();
    }

    QString toString() const;

    DBusIPCMessage createReply()
    {
        return DBusIPCMessage(m_message.createReply());
    }

    DBusIPCMessage createErrorReply(const QString &msg, const QString &member)
    {
        return DBusIPCMessage(m_message.createErrorReply(msg, member));
    }

    QString signature() const
    {
        return m_message.signature();
    }

    bool isReplyMessage() const
    {
        return (m_message.type() == QDBusMessage::ReplyMessage);
    }

    bool isErrorMessage() const
    {
        return (m_message.type() == QDBusMessage::ErrorMessage);
    }

    OutputPayLoad &outputPayLoad();

    InputPayLoad &inputPayLoad();

private:
    QDBusMessage m_message;
    std::unique_ptr<OutputPayLoad> m_outputPayload;
    std::unique_ptr<InputPayLoad> m_inputPayload;
};


class FaceliftIPCLibDBus_EXPORT OutputPayLoad
{

public:
    OutputPayLoad() : m_dataStream(&m_payloadArray, QIODevice::WriteOnly)
    {
    }

    template<typename Type>
    void writeSimple(const Type &v)
    {
        //        qCDebug(LogIpc) << "Writing to message : " << v;
        m_dataStream << v;
    }

    const QByteArray &getContent() const
    {
        return m_payloadArray;
    }

private:
    QByteArray m_payloadArray;
    QDataStream m_dataStream;
};

class FaceliftIPCLibDBus_EXPORT InputPayLoad
{

public:
    InputPayLoad(const QByteArray &payloadArray) : m_payloadArray(payloadArray), m_dataStream(m_payloadArray)
    {
    }

    ~InputPayLoad()
    {
        Q_ASSERT(m_dataStream.atEnd());
    }

    template<typename Type>
    void readNextParameter(Type &v)
    {
        m_dataStream >> v;
        //        qCDebug(LogIpc) << "Read from message : " << v;
    }

private:
    QByteArray m_payloadArray;
    QDataStream m_dataStream;
};




class DBusIPCServiceAdapterBase;

template<typename Type, typename Enable = void>
struct IPCTypeHandler
{
    static void writeDBUSSignature(QTextStream &s)
    {
        s << "i";
    }

    static void write(OutputPayLoad &msg, const Type &v)
    {
        msg.writeSimple(v);
    }

    static void read(InputPayLoad &msg, Type &v)
    {
        msg.readNextParameter(v);
    }

};



class DBusObjectRegistry;

class FaceliftIPCLibDBus_EXPORT DBusManager
{

public:
    DBusManager();

    static DBusManager &instance();

    bool isDBusConnected() const
    {
        return m_dbusConnected;
    }

    bool registerServiceName(const QString &serviceName);

    QDBusConnection &connection()
    {
        return m_busConnection;
    }

    QString serviceName() const
    {
        return m_busConnection.baseService();
    }

    DBusObjectRegistry &objectRegistry();

private:
    QDBusConnection m_busConnection;
    DBusObjectRegistry *m_objectRegistry = nullptr;
    bool m_dbusConnected;
};



class FaceliftIPCLibDBus_EXPORT DBusRequestHandler
{

public:

    virtual void deserializePropertyValues(DBusIPCMessage &msg, bool isCompleteSnapshot) = 0;
    virtual void deserializeSignal(DBusIPCMessage &msg) = 0;
    virtual void setServiceRegistered(bool isRegistered) = 0;

};


}

}
