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

#include "LocalIPC.h"
#include "IPCProxyNewBase.h"
#include "IPCProxyBase.h"

#if defined(FaceliftIPCLocalLib_LIBRARY)
#  define FaceliftIPCLocalLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCLocalLib_EXPORT Q_DECL_IMPORT
#endif

namespace facelift {

namespace local {

using namespace facelift;

class LocalIPCManager;
class LocalIPCServiceAdapterBase;

class FaceliftIPCLocalLib_EXPORT LocalIPCProxyBinder : public IPCProxyBinderBase
{
    Q_OBJECT

public:
    LocalIPCProxyBinder(InterfaceBase &owner, QObject *parent = nullptr);

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

    void onPropertiesChanged(LocalIPCMessage &message);

    void onSignalTriggered(LocalIPCMessage &message);

    void bindToIPC();

    void setServiceAvailable(bool isRegistered);

    void onServiceAvailable(LocalIPCServiceAdapterBase *adapter);

    void onServiceUnavailable();

    void notifyServiceAvailable();

    bool isServiceAvailable() const
    {
        return !m_serviceAdapter.isNull();
    }

    void requestPropertyValues();

    template<typename Type>
    void serializeValue(LocalIPCMessage &msg, const Type &v);

    template<typename Type>
    void deserializeValue(LocalIPCMessage &msg, Type &v);

    void onServerNotAvailableError(const char *methodName) const;

    template<typename PropertyType>
    void sendSetterCall(const char *methodName, const PropertyType &value);

    template<typename ... Args>
    LocalIPCMessage sendMethodCall(const char *methodName, const Args & ... args) const;

    template<typename ReturnType, typename ... Args>
    void sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<ReturnType> answer, const Args & ... args);

    template<typename ... Args>
    void sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<void> answer, const Args & ... args);

    template<typename ReturnType, typename ... Args>
    void sendMethodCallWithReturn(const char *methodName, ReturnType &returnValue, const Args & ... args) const;

    LocalIPCManager &manager() const;

    LocalIPCMessage call(LocalIPCMessage &message) const;

    void asyncCall(LocalIPCMessage &requestMessage, QObject *context, std::function<void(LocalIPCMessage &message)> callback);

    void setHandler(LocalIPCRequestHandler *handler)
    {
        m_serviceObject = handler;
        checkInit();
    }

private:
    QString m_serviceName;
    QString m_interfaceName;
    LocalIPCRequestHandler *m_serviceObject = nullptr;
    QPointer<LocalIPCServiceAdapterBase> m_serviceAdapter;
    QMetaObject::Connection m_signalConnection;
    bool m_explicitServiceName = false;
    bool m_serviceAvailable = false;
};


class FaceliftIPCLocalLib_EXPORT LocalIPCProxyBase : protected LocalIPCRequestHandler
{
public:
    LocalIPCProxyBase(LocalIPCProxyBinder &ipcBinder) : m_ipcBinder(ipcBinder)
    {
    }

protected:
    LocalIPCProxyBinder &m_ipcBinder;
    bool m_serviceRegistered = false;

};


template<typename InterfaceType>
class LocalIPCProxy : public IPCProxyBase<InterfaceType>, protected LocalIPCProxyBase
{

public:
    typedef const char *MemberIDType;
    using InputIPCMessage = ::facelift::local::LocalIPCMessage;
    using OutputIPCMessage = ::facelift::local::LocalIPCMessage;

    template<typename Type>
    using IPCProxyType = typename Type::IPCLocalProxyType;

    LocalIPCProxy(QObject *parent = nullptr) : IPCProxyBase<InterfaceType>(parent), LocalIPCProxyBase(m_ipcBinder), m_ipcBinder(*this)
    {
        m_ipcBinder.setInterfaceName(InterfaceType::FULLY_QUALIFIED_INTERFACE_NAME);
        m_ipcBinder.setHandler(this);

        this->initBinder(m_ipcBinder);
        this->setImplementationID("Local IPC Proxy");
    }

    template<typename T>
    MemberIDType memberID(T member, const char *memberName) const
    {
        // Local member IDs are strings. TODO : change to integer
        Q_UNUSED(member);
        return memberName;
    }

    template<typename Type>
    void serializeValue(LocalIPCMessage &msg, const Type &v)
    {
        typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
        IPCTypeHandler<SerializedType>::write(msg, IPCTypeRegisterHandler<Type>::convertToSerializedType(v, *this));
    }

    template<typename Type>
    void deserializeValue(LocalIPCMessage &msg, Type &v)
    {
        typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
        SerializedType serializedValue;
        IPCTypeHandler<SerializedType>::read(msg.inputPayLoad(), serializedValue);
        IPCTypeRegisterHandler<Type>::convertToDeserializedType(v, serializedValue, *this);
    }

    template<typename Type>
    bool deserializeOptionalValue(LocalIPCMessage &msg, Type &value, bool isCompleteSnapshot)
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

    void deserializePropertyValues(LocalIPCMessage &msg, bool isCompleteSnapshot) override
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

    LocalIPCProxyBinder *ipc()
    {
        return &m_ipcBinder;
    }

    template<typename SubInterfaceType>
    typename SubInterfaceType::IPCLocalProxyType *getOrCreateSubProxy(const QString &objectPath)
    {
        return m_ipcBinder.getOrCreateSubProxy<SubInterfaceType>(objectPath);
    }

    void connectToServer()
    {
        m_ipcBinder.connectToServer();
    }

private:
    LocalIPCProxyBinder m_ipcBinder;

};
}

}
