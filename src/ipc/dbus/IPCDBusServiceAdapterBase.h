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

#include <QDBusVirtualObject>
#include "IPCServiceAdapterBase.h"
#include "DBusIPCMessage.h"
#include "ipc-common.h"
#include "ipc-serialization.h"
#include "DBusManagerInterface.h"

namespace facelift {

namespace dbus {

class IPCDBusServiceAdapterBase : public IPCServiceAdapterBase
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

    void flush();

    template<typename Type>
    void serializeValue(DBusIPCMessage &msg, const Type &v);

    template<typename Type>
    void deserializeValue(DBusIPCMessage &msg, Type &v);

    void initOutgoingSignalMessage();

    template<typename MemberID, typename ... Args>
    void sendSignal(MemberID signalID, const Args & ... args);

    template<typename ReturnType>
    void sendAsyncCallAnswer(DBusIPCMessage &replyMessage, const ReturnType returnValue);

    void sendAsyncCallAnswer(DBusIPCMessage &replyMessage);

    void send(DBusIPCMessage &message);

    virtual IPCHandlingResult handleMethodCallMessage(DBusIPCMessage &requestMessage, DBusIPCMessage &replyMessage) = 0;

    virtual void serializePropertyValues(DBusIPCMessage &msg, bool isCompleteSnapshot);

    void registerService() override;

    void unregisterService() override;

    template<typename Type>
    void serializeOptionalValue(DBusIPCMessage &msg, const Type &currentValue, Type &previousValue, bool isCompleteSnapshot);

    template<typename Type>
    void serializeOptionalValue(DBusIPCMessage &msg, const Type &currentValue, bool isCompleteSnapshot);

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
    std::unique_ptr<DBusIPCMessage> m_pendingOutgoingMessage;
    DBusVirtualObject m_dbusVirtualObject;

    QString m_introspectionData;
    QString m_serviceName;

    bool m_previousReadyState = false;
    bool m_signalsConnected = false;
    bool m_alreadyInitialized = false;

    DBusManagerInterface& m_dbusManager;
};

template<typename Type>
inline void IPCDBusServiceAdapterBase::serializeValue(DBusIPCMessage &msg, const Type &v)
{
    typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
    IPCTypeHandler<SerializedType>::write(msg.outputPayLoad(), IPCTypeRegisterHandler<Type>::convertToSerializedType(v, *this));
}

template<typename Type>
inline void IPCDBusServiceAdapterBase::deserializeValue(DBusIPCMessage &msg, Type &v)
{
    typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
    SerializedType serializedValue;
    IPCTypeHandler<Type>::read(msg.inputPayLoad(), serializedValue);
    IPCTypeRegisterHandler<Type>::convertToDeserializedType(v, serializedValue, *this);
}

template<typename MemberID, typename ... Args>
inline void IPCDBusServiceAdapterBase::sendSignal(MemberID signalID, const Args & ... args)
{
    if (m_pendingOutgoingMessage == nullptr) {
        initOutgoingSignalMessage();
        auto argTuple = std::make_tuple(signalID, args ...);
        for_each_in_tuple(argTuple, SerializeParameterFunction<IPCDBusServiceAdapterBase>(m_pendingOutgoingMessage->outputPayLoad(), *this));
        flush();
    }
}

template<typename ReturnType>
inline void IPCDBusServiceAdapterBase::sendAsyncCallAnswer(DBusIPCMessage &replyMessage, const ReturnType returnValue)
{
    serializeValue(replyMessage, returnValue);
    send(replyMessage);
}

template<typename Type>
inline void IPCDBusServiceAdapterBase::serializeOptionalValue(DBusIPCMessage &msg, const Type &currentValue, Type &previousValue, bool isCompleteSnapshot)
{
    if (isCompleteSnapshot) {
        serializeValue(msg, currentValue);
    } else {
        if (previousValue == currentValue) {
            msg.outputPayLoad().writeSimple(false);
        } else {
            msg.outputPayLoad().writeSimple(true);
            serializeValue(msg, currentValue);
            previousValue = currentValue;
        }
    }
}

template<typename Type>
inline void IPCDBusServiceAdapterBase::serializeOptionalValue(DBusIPCMessage &msg, const Type &currentValue, bool isCompleteSnapshot)
{
    msg.outputPayLoad().writeSimple(isCompleteSnapshot);
    if (isCompleteSnapshot) {
        serializeValue(msg, currentValue);
    }
}

} // end namespace dbus
} // end namespace facelift
