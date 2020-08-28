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

#include <QDBusVirtualObject>
#include "IPCServiceAdapterBase.h"
#include "DBusIPCMessage.h"
#include "DBusIPCCommon.h"
#include "ipc-common.h"
#include "FaceliftUtils.h"
#include "DBusManagerInterface.h"

namespace facelift {

namespace dbus {

class FaceliftIPCLibDBus_EXPORT IPCDBusServiceAdapterBase : public IPCServiceAdapterBase
{
    Q_OBJECT

    typedef const char *MemberIDType;

public:

    template<typename Type>
    using IPCAdapterType = typename Type::IPCDBusAdapterType;

    class DBusVirtualObject : public QDBusVirtualObject
    {

    public:
        DBusVirtualObject(IPCDBusServiceAdapterBase &adapter) : QDBusVirtualObject(nullptr), m_adapter(adapter)
        {
        }

        QString introspect(const QString &path) const override
        {
            return m_adapter.introspect(path);
        }

        bool handleMessage(const QDBusMessage &message, const QDBusConnection &connection) override
        {
            Q_UNUSED(connection);
            return m_adapter.handleMessage(message);
        }

    private:
        IPCDBusServiceAdapterBase &m_adapter;
    };

    IPCDBusServiceAdapterBase(DBusManagerInterface& dbusManager, QObject *parent = nullptr);

    ~IPCDBusServiceAdapterBase();

    bool handleMessage(const QDBusMessage &dbusMsg);


    template<typename Value>
    inline void sendPropertiesChanged(const QString& property , const Value & value);

    template<typename ... Args>
    void sendSignal(const QString& signalName, const Args & ... args);

    template<typename ReturnType>
    void sendAsyncCallAnswer(DBusIPCMessage &replyMessage, const ReturnType returnValue);

    void sendAsyncCallAnswer(DBusIPCMessage &replyMessage);

    void send(DBusIPCMessage &message);

    virtual IPCHandlingResult handleMethodCallMessage(DBusIPCMessage &requestMessage, DBusIPCMessage &replyMessage) = 0;

    virtual void marshalPropertyValues(const QList<QVariant>& arguments, DBusIPCMessage &msg) = 0;

    virtual void marshalProperty(const QList<QVariant>& arguments, DBusIPCMessage &msg) = 0;

    virtual void setProperty(const QList<QVariant>& arguments) = 0;

    void registerService() override;

    void unregisterService() override;

    virtual void appendDBUSIntrospectionData(QTextStream &s) const = 0;

    QString introspect(const QString &path) const;

    template<typename T>
    MemberIDType memberID(T member, MemberIDType memberName) const
    {
        // DBus member IDs are strings
        Q_UNUSED(member);
        return memberName;
    }

protected:
    DBusVirtualObject m_dbusVirtualObject;

    QString m_introspectionData;
    QString m_serviceName;

    bool m_previousReadyState = false;
    bool m_signalsConnected = false;
    bool m_alreadyInitialized = false;

    DBusManagerInterface& m_dbusManager;
};

template<typename Value>
inline void IPCDBusServiceAdapterBase::sendPropertiesChanged(const QString& property, const Value &value)
{
    DBusIPCMessage reply(objectPath(), DBusIPCCommon::PROPERTIES_INTERFACE_NAME, DBusIPCCommon::PROPERTIES_CHANGED_SIGNAL_NAME);
    reply << interfaceName();
    reply << QVariantMap{{property, QVariant::fromValue(value)}};
    this->send(reply);
}

template<>
inline void IPCDBusServiceAdapterBase::sendPropertiesChanged(const QString& property, const QList<QString> &value)
{
    DBusIPCMessage reply(objectPath(), DBusIPCCommon::PROPERTIES_INTERFACE_NAME, DBusIPCCommon::PROPERTIES_CHANGED_SIGNAL_NAME);
    reply << interfaceName();
    reply << QVariantMap{{property, QVariant::fromValue(QStringList(value))}};
    this->send(reply);
}

template<typename ... Args>
inline void IPCDBusServiceAdapterBase::sendSignal(const QString& signalName, const Args & ... args)
{
    DBusIPCMessage signal(objectPath(), interfaceName(), signalName);
    auto argTuple = std::make_tuple(args ...);
    for_each_in_tuple(argTuple, [this, &signal](const auto &v){signal << QVariant::fromValue(v);});
    this->send(signal);
}

template<typename ReturnType>
inline void IPCDBusServiceAdapterBase::sendAsyncCallAnswer(DBusIPCMessage &replyMessage, const ReturnType returnValue)
{
    replyMessage << QVariant::fromValue(returnValue);
    send(replyMessage);
}

} // end namespace dbus
} // end namespace facelift
