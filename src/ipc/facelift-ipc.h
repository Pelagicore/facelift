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

#include "ipc-common/IPCServiceAdapterBase.h"
#include "ipc-common/IPCProxyBase.h"

namespace facelift {

template<typename InterfaceType>
class IPCServiceAdapter : public NewIPCServiceAdapterBase
{
public:
    using TheServiceType = InterfaceType;

    IPCServiceAdapter(QObject *parent) : NewIPCServiceAdapterBase(parent)
    {
        setObjectPath(InterfaceType::SINGLETON_OBJECT_PATH);
    }

    InterfaceBase *service() const override
    {
        return m_service;
    }

    void init() override
    {
        registerLocalService();

#ifdef DBUS_IPC_ENABLED
        m_ipcAdapter.setObjectPath(objectPath());
        m_ipcAdapter.setService(m_service);
        m_ipcAdapter.init();
#endif
    }

    void setService(QObject *service) override
    {
        m_service = bindToProvider<InterfaceType>(service);
    }

private:
    QPointer<InterfaceType> m_service;

#ifdef DBUS_IPC_ENABLED
    using IPCDBusAdapterType = typename InterfaceType::IPCDBusAdapterType;
    IPCDBusAdapterType m_ipcAdapter;
#endif

};

template<typename WrapperType, typename NotAvailableImpl>
class IPCProxy : public WrapperType, public IPCProxyNewBase
{
    using InterfaceType = typename NotAvailableImpl::InterfaceType;

public:
    IPCProxy(QObject *parent) : WrapperType(parent)
        , IPCProxyNewBase(*static_cast<InterfaceBase *>(this))
        , m_localProviderBinder(*this)
    {
        QObject::connect(ipc(), &IPCProxyBinderBase::complete, this, [this] () {
                m_localProviderBinder.init();
#ifdef DBUS_IPC_ENABLED
                m_ipcProxy.ipc()->setObjectPath(ipc()->objectPath());
                m_ipcProxy.connectToServer();
                QObject::connect(m_ipcProxy.ipc(), &IPCProxyBinderBase::serviceAvailableChanged, this, &IPCProxy::refreshProvider);
#endif
                this->refreshProvider();
            });

        refreshProvider();
    }

    void refreshProvider()
    {
        InterfaceType *provider = &m_notAvailableProvider;
        if (m_localProviderBinder.provider() != nullptr) {
            provider = m_localProviderBinder.provider();
#ifdef DBUS_IPC_ENABLED
        } else if (m_ipcProxy.ipc()->isServiceAvailable()) {
            provider = &m_ipcProxy;
#endif
        }

        this->setWrapped(provider);
    }

    void connectToServer()
    {
        emit this->componentCompleted();
    }

private:
    NotAvailableImpl m_notAvailableProvider;
    LocalProviderBinder<InterfaceType> m_localProviderBinder;
#ifdef DBUS_IPC_ENABLED
    using IPCProxyType = typename NotAvailableImpl::IPCDBusProxyType;
    IPCProxyType m_ipcProxy;
#endif
};

}
