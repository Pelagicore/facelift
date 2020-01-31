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

#include <array>
#include <QObject>
#include "StaticArrayReference.h"
#include "IPCProxyBinderBase.h"
#include "LocalProviderBinder.h"

#if defined(FaceliftIPCCommonLib_LIBRARY)
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_IMPORT
#endif

namespace facelift {

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
                auto serviceName = ipc()->serviceName();
                auto objPath = ipc()->objectPath();
                for (auto& proxy : m_ipcProxies) {
                    auto proxyAdapterIPCBinder = proxy.ipcBinder;
                    if (proxyAdapterIPCBinder != nullptr) {
                        proxyAdapterIPCBinder->setObjectPath(objPath);
                        if(!serviceName.isEmpty()) {
                            proxyAdapterIPCBinder->setServiceName(serviceName);
                        }
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
                    break;
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
    static ProxyAdapterEntry createIPCProxy(ProxyType &proxy)
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
