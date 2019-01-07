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
#include "ipc-common/IPCProxyBase.h"

#if defined(FaceliftIPCLibDBus_LIBRARY)
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_IMPORT
#endif

namespace facelift {

namespace dbus {

using namespace facelift;


class FaceliftIPCLibDBus_EXPORT DBusIPCProxyBinder : public IPCProxyBinderBase
{
    Q_OBJECT

public:

    DBusIPCProxyBinder(InterfaceBase &owner, QObject *parent = nullptr) : IPCProxyBinderBase(owner, parent)
    {
        m_busWatcher.setWatchMode(QDBusServiceWatcher::WatchForRegistration);
    }

    const QString &serviceName() const
    {
        return m_serviceName;
    }

    void setServiceName(const QString &name)
    {
        m_serviceName = name;
        m_explicitServiceName = true;
        checkInit();
    }

    const QString &interfaceName() const
    {
        return m_interfaceName;
    }

    void setInterfaceName(const QString &name)
    {
        m_interfaceName = name;
        checkInit();
    }

    Q_SLOT
    void onPropertiesChanged(const QDBusMessage &dbusMessage)
    {
        DBusIPCMessage msg(dbusMessage);
        m_serviceObject->deserializePropertyValues(msg, false);
    }

    Q_SLOT
    void onSignalTriggered(const QDBusMessage &dbusMessage)
    {
        DBusIPCMessage msg(dbusMessage);
        m_serviceObject->deserializePropertyValues(msg, false);
        m_serviceObject->deserializeSignal(msg);
    }

    void bindToIPC();

    void onServiceAvailable();

    void setServiceAvailable(bool isRegistered)
    {
        if (m_serviceAvailable != isRegistered) {
            m_serviceAvailable = isRegistered;
            emit serviceAvailableChanged();
        }
    }

    bool isServiceAvailable() const
    {
        return m_serviceAvailable;
    }

    bool m_serviceAvailable = false;

    void requestPropertyValues()
    {
        DBusIPCMessage msg(serviceName(), objectPath(), interfaceName(), DBusIPCCommon::GET_PROPERTIES_MESSAGE_NAME);

        auto replyHandler = [this](DBusIPCMessage &replyMessage) {
            if (replyMessage.isReplyMessage()) {
                m_serviceObject->deserializePropertyValues(replyMessage, true);
                m_serviceObject->setServiceRegistered(true);
            } else {
                qDebug() << "Service not yet available : " << objectPath();
            }
        };

        if (isSynchronous()) {
            auto replyMessage = msg.call(connection());
            replyHandler(replyMessage);
        } else {
            msg.asyncCall(connection(), this, replyHandler);
        }
    }

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
        IPCTypeHandler<SerializedType>::read(msg.inputPayLoad(), serializedValue);
        IPCTypeRegisterHandler<Type>::convertToDeserializedType(v, serializedValue, *this);
    }

    void onServerNotAvailableError(const char *methodName) const
    {
        qCritical(
            "Error message received when calling method '%s' on service at path '%s'. "
            "This likely indicates that the server you are trying to access is not available yet",
            qPrintable(methodName), qPrintable(objectPath()));
    }

    template<typename PropertyType>
    void sendSetterCall(const char *methodName, const PropertyType &value)
    {
        DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
        serializeValue(msg, value);
        if (isSynchronous()) {
            auto replyMessage = msg.call(connection());
            if (replyMessage.isErrorMessage()) {
                onServerNotAvailableError(methodName);
            }
        } else {
            msg.asyncCall(connection(), this, [this, methodName](const DBusIPCMessage &replyMessage) {
                if (replyMessage.isErrorMessage()) {
                    onServerNotAvailableError(methodName);
                }
            });
        }
    }

    struct SerializeParameterFunction
    {
        SerializeParameterFunction(DBusIPCMessage &msg, const DBusIPCProxyBinder &parent) :
            m_msg(msg),
            m_parent(parent)
        {
        }

        DBusIPCMessage &m_msg;
        const DBusIPCProxyBinder &m_parent;

        template<typename Type>
        void operator()(const Type &v)
        {
            IPCTypeHandler<typename IPCTypeRegisterHandler<Type>::SerializedType>::write(m_msg.outputPayLoad(),
                    IPCTypeRegisterHandler<Type>::convertToSerializedType(v, m_parent));
        }
    };

    template<typename ... Args>
    DBusIPCMessage sendMethodCall(const char *methodName, const Args & ... args) const
    {
        DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
        auto argTuple = std::make_tuple(args ...);
        for_each_in_tuple(argTuple, SerializeParameterFunction(msg, *this));
        auto replyMessage = msg.call(connection());
        if (replyMessage.isErrorMessage()) {
            onServerNotAvailableError(methodName);
        }
        return replyMessage;
    }

    template<typename ReturnType, typename ... Args>
    void sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<ReturnType> answer, const Args & ... args)
    {
        DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
        auto argTuple = std::make_tuple(args ...);
        for_each_in_tuple(argTuple, SerializeParameterFunction(msg, *this));
        msg.asyncCall(connection(), this, [this, answer](DBusIPCMessage &msg) {
            ReturnType returnValue;
            if (msg.isReplyMessage()) {
                deserializeValue(msg, returnValue);
                answer(returnValue);
            } else {
                qWarning() << "Error received" << msg.toString();
            }
        });
    }

    template<typename ... Args>
    void sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<void> answer, const Args & ... args)
    {
        DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
        auto argTuple = std::make_tuple(args ...);
        for_each_in_tuple(argTuple, SerializeParameterFunction(msg, *this));
        msg.asyncCall(connection(), this, [answer](DBusIPCMessage &msg) {
            Q_UNUSED(msg);
            answer();
        });
    }

    QDBusConnection &connection() const
    {
        return manager().connection();
    }

    DBusManager &manager() const
    {
        return DBusManager::instance();
    }

    void setHandler(DBusRequestHandler *handler)
    {
        m_serviceObject = handler;
        checkInit();
    }

private:
    QString m_serviceName;
    QString m_interfaceName;

    DBusRequestHandler *m_serviceObject = nullptr;

    QDBusServiceWatcher m_busWatcher;
    bool m_explicitServiceName = false;

};



template<typename InterfaceType>
class DBusIPCProxy : public IPCProxyBase<InterfaceType>, protected DBusRequestHandler
{
    using IPCProxyBase<InterfaceType>::assignDefaultValue;

public:
    typedef const char *MemberIDType;

    DBusIPCProxy(QObject *parent = nullptr) : IPCProxyBase<InterfaceType>(parent), m_ipcBinder(*this)
    {
        m_ipcBinder.setInterfaceName(InterfaceType::FULLY_QUALIFIED_INTERFACE_NAME);
        m_ipcBinder.setHandler(this);

        this->initBinder(m_ipcBinder);
        this->setImplementationID("DBus IPC Proxy");
    }

    template<typename T>
    MemberIDType memberID(T member, const char *memberName) const
    {
        // DBus member IDs are strings
        Q_UNUSED(member);
        return memberName;
    }

    template<typename Type>
    void serializeValue(DBusIPCMessage &msg, const Type &v)
    {
        typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
        IPCTypeHandler<SerializedType>::write(msg, IPCTypeRegisterHandler<Type>::convertToSerializedType(v, *this));
    }

    template<typename Type>
    void deserializeValue(DBusIPCMessage &msg, Type &v)
    {
        typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
        SerializedType serializedValue;
        IPCTypeHandler<SerializedType>::read(msg.inputPayLoad(), serializedValue);
        IPCTypeRegisterHandler<Type>::convertToDeserializedType(v, serializedValue, *this);
    }

    template<typename Type>
    bool deserializeOptionalValue(DBusIPCMessage &msg, Type &value, bool isCompleteSnapshot)
    {
        bool b = true;
        if (!isCompleteSnapshot) {
            msg.inputPayLoad().readNextParameter(b);
        }
        if (b) {
            this->deserializeValue(msg, value);
        }
        return b;
    }

    void deserializePropertyValues(DBusIPCMessage &msg, bool isCompleteSnapshot) override
    {
        Q_UNUSED(isCompleteSnapshot);
        bool isReady = false;
        deserializeValue(msg, isReady);
        this->setServiceReady(isReady);
    }

    void setServiceRegistered(bool isRegistered) override
    {
        bool oldReady = this->ready();
        m_serviceRegistered = isRegistered;
        if (this->ready() != oldReady) {
            this->readyChanged();
        }
        this->emitChangeSignals();

        m_ipcBinder.setServiceAvailable(isRegistered);
    }

    template<typename PropertyType>
    void sendSetterCall(const char *methodName, const PropertyType &value)
    {
        m_ipcBinder.sendSetterCall(methodName, value);
    }

    template<typename ... Args>
    void sendMethodCall(const char *methodName, const Args & ... args) const
    {
        m_ipcBinder.sendMethodCall(methodName, args ...);
    }

    template<typename ReturnType, typename ... Args>
    void sendMethodCallWithReturn(const char *methodName, ReturnType &returnValue, const Args & ... args) const
    {
        DBusIPCMessage msg = m_ipcBinder.sendMethodCall(methodName, args ...);
        if (msg.isReplyMessage()) {
            const_cast<DBusIPCProxy *>(this)->deserializeValue(msg, returnValue);
        } else {
            assignDefaultValue(returnValue);
        }
    }

    template<typename ReturnType, typename ... Args>
    void sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<ReturnType> answer, const Args & ... args) const
    {
        const_cast<DBusIPCProxy *>(this)->m_ipcBinder.sendAsyncMethodCall(methodName, answer, args ...);
    }

    DBusIPCProxyBinder *ipc()
    {
        return &m_ipcBinder;
    }

    template<typename InterfaceType_>
    typename InterfaceType_::IPCDBusProxyType *getOrCreateSubProxy(const QString &objectPath)
    {
        return m_ipcBinder.getOrCreateSubProxy<InterfaceType_>(objectPath);
    }

    void connectToServer()
    {
        m_ipcBinder.connectToServer();
    }

private:
    DBusIPCProxyBinder m_ipcBinder;
    bool m_serviceRegistered = false;

};
}

}
