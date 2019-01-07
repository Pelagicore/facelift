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

#include "ipc-dbus.h"
#include "ipc-common/IPCServiceAdapterBase.h"


#if defined(FaceliftIPCLibDBus_LIBRARY)
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_IMPORT
#endif

namespace facelift {

namespace dbus {

using namespace facelift;

class FaceliftIPCLibDBus_EXPORT DBusIPCServiceAdapterBase : public IPCServiceAdapterBase
{
    Q_OBJECT

public:

    class DBusVirtualObject : public QDBusVirtualObject
    {

    public:
        DBusVirtualObject(DBusIPCServiceAdapterBase &adapter) : QDBusVirtualObject(nullptr), m_adapter(adapter)
        {
        }

        QString introspect(const QString &path) const override
        {
            return m_adapter.introspect(path);
        }

        bool handleMessage(const QDBusMessage &message, const QDBusConnection &connection) override
        {
            return m_adapter.handleMessage(message, connection);
        }

    private:
        DBusIPCServiceAdapterBase &m_adapter;
    };

    DBusIPCServiceAdapterBase(QObject *parent = nullptr) : IPCServiceAdapterBase(parent), m_dbusVirtualObject(*this)
    {
    }

    ~DBusIPCServiceAdapterBase();

    virtual QString introspect(const QString &path) const = 0;

    bool handleMessage(const QDBusMessage &dbusMsg, const QDBusConnection &connection);

    void flush()
    {
        if (m_pendingOutgoingMessage) {
            m_pendingOutgoingMessage->send(dbusManager().connection());
            m_pendingOutgoingMessage.reset();
        }
    }

    struct SerializeParameterFunction
    {
        SerializeParameterFunction(OutputPayLoad &msg, DBusIPCServiceAdapterBase &parent) :
            m_msg(msg),
            m_parent(parent)
        {
        }

        OutputPayLoad &m_msg;
        DBusIPCServiceAdapterBase &m_parent;

        template<typename Type>
        void operator()(const Type &v)
        {
            IPCTypeHandler<typename IPCTypeRegisterHandler<Type>::SerializedType>::write(m_msg,
                    IPCTypeRegisterHandler<Type>::convertToSerializedType(v, m_parent));
        }
    };

    template<typename Type>
    void serializeValue(DBusIPCMessage &msg, const Type &v)
    {
        typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
        IPCTypeHandler<SerializedType>::write(msg.outputPayLoad(), IPCTypeRegisterHandler<Type>::convertToSerializedType(v, *this));
    }

    template<typename Type>
    void deserializeValue(DBusIPCMessage &msg, Type &v)
    {
        typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
        SerializedType serializedValue;
        IPCTypeHandler<Type>::read(msg.inputPayLoad(), serializedValue);
        IPCTypeRegisterHandler<Type>::convertToDeserializedType(v, serializedValue, *this);
    }

    template<typename MemberID, typename ... Args>
    void sendSignal(MemberID signalID, const Args & ... args)
    {
        if (m_pendingOutgoingMessage == nullptr) {
            m_pendingOutgoingMessage = std::make_unique<DBusIPCMessage>(objectPath(), interfaceName(), DBusIPCCommon::SIGNAL_TRIGGERED_SIGNAL_NAME);

            // Send property value updates before the signal itself so that they are set before the signal is triggered on the client side.
            this->serializePropertyValues(*m_pendingOutgoingMessage, false);

            auto argTuple = std::make_tuple(signalID, args ...);
            for_each_in_tuple(argTuple, SerializeParameterFunction(m_pendingOutgoingMessage->outputPayLoad(), *this));
            flush();
        }
    }

    template<typename ReturnType>
    void sendAsyncCallAnswer(DBusIPCMessage &replyMessage, const ReturnType returnValue)
    {
        serializeValue(replyMessage, returnValue);
        replyMessage.send(dbusManager().connection());
    }

    void sendAsyncCallAnswer(DBusIPCMessage &replyMessage)
    {
        replyMessage.send(dbusManager().connection());
    }

    template<typename Type>
    void addPropertySignature(QTextStream &s, const char *propertyName, bool isReadonly) const
    {
        s << "<property name=\"" << propertyName << "\" type=\"";
        std::tuple<Type> dummyTuple;
        appendDBUSTypeSignature(s, dummyTuple);
        s << "\" access=\"" << (isReadonly ? "read" : "readwrite") << "\"/>";
    }

    template<typename ... Args>
    void addMethodSignature(QTextStream &s, const char *methodName,
            const std::array<const char *, sizeof ... (Args)> &argNames) const
    {
        s << "<method name=\"" << methodName << "\">";
        std::tuple<Args ...> t;  // TODO : get rid of the tuple
        appendDBUSMethodArgumentsSignature(s, t, argNames);
        s << "</method>";
    }

    template<typename ... Args>
    void addSignalSignature(QTextStream &s, const char *methodName,
            const std::array<const char *, sizeof ... (Args)> &argNames) const
    {
        s << "<signal name=\"" << methodName << "\">";
        std::tuple<Args ...> t;  // TODO : get rid of the tuple
        appendDBUSSignalArgumentsSignature(s, t, argNames);
        s << "</signal>";
    }

    virtual IPCHandlingResult handleMethodCallMessage(DBusIPCMessage &requestMessage, DBusIPCMessage &replyMessage) = 0;

    virtual void serializePropertyValues(DBusIPCMessage &msg, bool isCompleteSnapshot)
    {
        Q_UNUSED(isCompleteSnapshot);
        Q_ASSERT(service());
        serializeValue(msg, service()->ready());
    }

    void init();

    DBusManager &dbusManager()
    {
        return DBusManager::instance();
    }

    template<typename Type>
    void serializeOptionalValue(DBusIPCMessage &msg, const Type &currentValue, Type &previousValue, bool isCompleteSnapshot)
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
    void serializeOptionalValue(DBusIPCMessage &msg, const Type &currentValue, bool isCompleteSnapshot)
    {
        msg.outputPayLoad().writeSimple(isCompleteSnapshot);
        if (isCompleteSnapshot) {
            serializeValue(msg, currentValue);
        }
    }


protected:
    std::unique_ptr<DBusIPCMessage> m_pendingOutgoingMessage;
    DBusVirtualObject m_dbusVirtualObject;

    QString m_introspectionData;
    QString m_serviceName;

    bool m_alreadyInitialized = false;
};



template<typename ServiceType>
class DBusIPCServiceAdapter : public DBusIPCServiceAdapterBase
{
public:
    typedef ServiceType TheServiceType;
    typedef const char *MemberIDType;

    DBusIPCServiceAdapter(QObject *parent) : DBusIPCServiceAdapterBase(parent)
    {
        setInterfaceName(ServiceType::FULLY_QUALIFIED_INTERFACE_NAME);
    }

    template<typename T>
    MemberIDType memberID(T member, const char *memberName) const
    {
        // DBus member IDs are strings
        Q_UNUSED(member);
        return memberName;
    }

    ServiceType *service() const override
    {
        return m_service;
    }

    void setService(ServiceType *service)
    {
        m_service = service;
    }

    virtual void appendDBUSIntrospectionData(QTextStream &s) const = 0;

    QString introspect(const QString &path) const override
    {
        QString introspectionData;

        if (path == objectPath()) {
            QTextStream s(&introspectionData);
            s << "<interface name=\"" << interfaceName() << "\">";
            appendDBUSIntrospectionData(s);
            s << "</interface>";
        } else {
            qFatal("Wrong object path");
        }

        qDebug() << "Introspection data for " << path << ":" << introspectionData;
        return introspectionData;
    }

protected:
    QPointer<ServiceType> m_service;
};

}

}
