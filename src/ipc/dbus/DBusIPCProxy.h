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
#include "IPCProxyBase.h"
#include <QDBusServiceWatcher>

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

    DBusIPCProxyBinder(InterfaceBase &owner, QObject *parent = nullptr) : IPCProxyBinderBase(owner, parent), m_registry(DBusManager::instance().objectRegistry())
    {
        m_busWatcher.setWatchMode(QDBusServiceWatcher::WatchForRegistration);
    }

    const QString &serviceName() const
    {
        return m_serviceName;
    }

    void setServiceName(const QString &name);

    const QString &interfaceName() const
    {
        return m_interfaceName;
    }

    void setInterfaceName(const QString &name);

    Q_SLOT void onPropertiesChanged(const QDBusMessage &dbusMessage);

    Q_SLOT void onSignalTriggered(const QDBusMessage &dbusMessage);

    void bindToIPC();

    void onServiceNameKnown();

    void setServiceAvailable(bool isRegistered);

    bool isServiceAvailable() const
    {
        return m_serviceAvailable;
    }

    void requestPropertyValues();

    template<typename Type>
    void serializeValue(DBusIPCMessage &msg, const Type &v);

    template<typename Type>
    void deserializeValue(DBusIPCMessage &msg, Type &v);

    void onServerNotAvailableError(const char *methodName) const;

    template<typename PropertyType>
    void sendSetterCall(const char *methodName, const PropertyType &value);

    template<typename ... Args>
    DBusIPCMessage sendMethodCall(const char *methodName, const Args & ... args) const;

    template<typename ReturnType, typename ... Args>
    void sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<ReturnType> answer, const Args & ... args);

    template<typename ... Args>
    void sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<void> answer, const Args & ... args);

    template<typename ReturnType, typename ... Args>
    void sendMethodCallWithReturn(const char *methodName, ReturnType &returnValue, const Args & ... args) const;

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
    void checkRegistry();

    QString m_serviceName;
    QString m_interfaceName;
    QDBusServiceWatcher m_busWatcher;
    DBusRequestHandler *m_serviceObject = nullptr;
    bool m_explicitServiceName = false;
    bool m_serviceAvailable = false;
    DBusObjectRegistry &m_registry;
};


class FaceliftIPCLibDBus_EXPORT DBusIPCProxyBase : protected DBusRequestHandler
{
public:
    DBusIPCProxyBase(DBusIPCProxyBinder& ipcBinder) : m_ipcBinder(ipcBinder) {
    }

protected:
    DBusIPCProxyBinder& m_ipcBinder;
    bool m_serviceRegistered = false;

};


template<typename InterfaceType>
class DBusIPCProxy : public IPCProxyBase<InterfaceType>, protected DBusIPCProxyBase
{

public:
    typedef const char *MemberIDType;
    using InputIPCMessage = ::facelift::dbus::DBusIPCMessage;
    using OutputIPCMessage = ::facelift::dbus::DBusIPCMessage;

    DBusIPCProxy(QObject *parent = nullptr) : IPCProxyBase<InterfaceType>(parent), DBusIPCProxyBase(m_ipcBinder), m_ipcBinder(*this)
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

    DBusIPCProxyBinder *ipc()
    {
        return &m_ipcBinder;
    }

    template<typename SubInterfaceType>
    typename SubInterfaceType::IPCDBusProxyType *getOrCreateSubProxy(const QString &objectPath)
    {
        return m_ipcBinder.getOrCreateSubProxy<SubInterfaceType>(objectPath);
    }

    void connectToServer()
    {
        m_ipcBinder.connectToServer();
    }

private:
    DBusIPCProxyBinder m_ipcBinder;

};
}

}
