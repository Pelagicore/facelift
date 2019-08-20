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

#include "IPCServiceAdapterBase.h"
#include "IPCProxyNewBase.h"

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

    ~IPCServiceAdapter() {
        unregisterLocalService();
    }

    InterfaceBase *service() const override
    {
        return m_service;
    }

    void registerService() override
    {
        registerLocalService();
        for (auto& ipcAdapter : m_ipcServiceAdapters) {
            ipcAdapter->registerService(objectPath(), m_service);
        }
    }

    void unregisterService() override
    {
        unregisterLocalService();
    }

    void setService(QObject *service) override
    {
        m_service = bindToProvider<InterfaceType>(service);
    }

    void addServiceAdapter(IPCServiceAdapterBase &adapter) {
        m_ipcServiceAdapters.append(&adapter);
    }

private:
    QPointer<InterfaceType> m_service;
    QList<IPCServiceAdapterBase*> m_ipcServiceAdapters;

};


template<typename Type>
class StaticArrayReference
{
public:

    StaticArrayReference()
    {
    }

    template<size_t SIZE>
    StaticArrayReference(const std::array<Type, SIZE>& array)
    {
        m_data = array.data();
        m_size = SIZE;
    }

    const Type* begin() const
    {
        return m_data;
    }

    const Type* end() const
    {
        return m_data + m_size;
    }

    void reset()
    {
        m_size = 0;
    }

private:
    const Type* m_data = nullptr;
    size_t m_size = 0;
};



template<typename WrapperType, typename NotAvailableImpl>
class IPCProxy : public WrapperType, public IPCProxyNewBase
{
    using InterfaceType = typename NotAvailableImpl::InterfaceType;

public:

    struct ProxyAdapterEntry {
        IPCProxyBinderBase* ipcBinder = nullptr;
        InterfaceType* proxy = nullptr;
    };

    IPCProxy(QObject *parent) : WrapperType(parent)
        , IPCProxyNewBase(*static_cast<InterfaceBase *>(this))
        , m_localProviderBinder(*this)
    {
        QObject::connect(ipc(), &IPCProxyBinderBase::complete, this, [this] () {
                m_localProviderBinder.init();

                for (auto& proxy : m_ipcProxies) {
                    auto proxyAdapterIPCBinder = proxy.ipcBinder;
                    if (proxyAdapterIPCBinder != nullptr) {
                        proxyAdapterIPCBinder->setObjectPath(ipc()->objectPath());
                        proxyAdapterIPCBinder->connectToServer();
                        QObject::connect(proxyAdapterIPCBinder, &IPCProxyBinderBase::serviceAvailableChanged, this, &IPCProxy::refreshProvider);
                    }
                }
                this->refreshProvider();
            });

        refreshProvider();
    }

    void refreshProvider() override
    {
        InterfaceType *provider = &m_notAvailableProvider;
        if (m_localProviderBinder.provider() != nullptr) {
            provider = m_localProviderBinder.provider();
        } else {
            for (auto& proxy : m_ipcProxies) {
                auto proxyAdapterIPCBinder = proxy.ipcBinder;
                if ((proxyAdapterIPCBinder != nullptr) && proxyAdapterIPCBinder->isServiceAvailable()) {
                    provider = proxy.proxy;
                }
            }
        }

        this->setWrapped(provider);
    }

    void connectToServer()
    {
        emit this->componentCompleted();
    }

    template<typename ProxyType>
    ProxyAdapterEntry createIPCAdapter(ProxyType &proxy)
    {
        ProxyAdapterEntry entry;
        entry.ipcBinder = proxy.ipc();
        entry.proxy = &proxy;
        return entry;
    }

    template<size_t N>
    void setIPCProxies(std::array<ProxyAdapterEntry, N>& proxies)
    {
        m_ipcProxies = proxies;
    }

    void resetIPCProxies()
    {
        m_ipcProxies.reset();
    }

private:
    NotAvailableImpl m_notAvailableProvider;
    LocalProviderBinder<InterfaceType> m_localProviderBinder;
    StaticArrayReference<ProxyAdapterEntry> m_ipcProxies;
};

}
