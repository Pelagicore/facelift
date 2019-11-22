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

#include <QObject>
#include "InterfaceManager.h"
#include "IPCProxyNewBase.h"

#if defined(FaceliftIPCCommonLib_LIBRARY)
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_IMPORT
#endif

namespace facelift {


template<typename InterfaceType>
class LocalProviderBinder : public QObject
{

public:
    LocalProviderBinder(IPCProxyNewBase &proxy) : m_proxy(proxy)
    {
        QObject::connect(&m_interfaceManager, &InterfaceManager::adapterAvailable, this,
                &LocalProviderBinder::onLocalAdapterAvailable);
        QObject::connect(&m_interfaceManager, &InterfaceManager::adapterUnavailable, this,
                &LocalProviderBinder::onLocalAdapterUnavailable);
    }

    void init()
    {
        auto localAdapter = m_interfaceManager.getAdapter(m_proxy.objectPath());
        if (localAdapter) {
            onLocalAdapterAvailable(localAdapter);
        }
    }

    void onLocalAdapterUnavailable(QString objectPath, NewIPCServiceAdapterBase *adapter)
    {
        Q_UNUSED(objectPath);
        if (m_adapter == adapter) {  // We reset if the unregistered instance is the one we were bound to
            m_provider = nullptr;
            m_adapter = nullptr;
            m_proxy.refreshProvider();
        }
    }

    void onLocalAdapterAvailable(NewIPCServiceAdapterBase *adapter)
    {
        auto* service = m_interfaceManager.serviceMatches(m_proxy.objectPath(), adapter);
        if (service) {
            auto provider = qobject_cast<InterfaceType *>(service);
            m_provider = provider;
            m_adapter = adapter;
            if (m_provider) {
                qCDebug(LogIpc) << "Local server found for " << m_proxy.objectPath();
                m_proxy.refreshProvider();
            }
        }
    }

    InterfaceType *provider()
    {
        return m_provider;
    }

private:
    QPointer<InterfaceType> m_provider;
    QPointer<NewIPCServiceAdapterBase> m_adapter;
    IPCProxyNewBase &m_proxy;
    InterfaceManager &m_interfaceManager = InterfaceManager::instance();
};

}
