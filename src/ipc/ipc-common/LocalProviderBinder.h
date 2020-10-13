/**********************************************************************
**
** Copyright (C) 2020 Luxoft Sweden AB
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
#include "LocalProviderBinderBase.h"
#include "IPCProxyNewBase.h"
#include "InterfaceManagerInterface.h"
#include "NewIPCServiceAdapterBase.h"

namespace facelift {

template<typename InterfaceType>
class LocalProviderBinder : public LocalProviderBinderBase
{

public:
    LocalProviderBinder(InterfaceManagerInterface& interfaceManager, IPCProxyNewBase &proxy) :
        LocalProviderBinderBase(interfaceManager, proxy)
    {
    }

    void checkLocalAdapterAvailability() override
    {
        auto adapter = m_interfaceManager.getAdapter(m_proxy.objectPath());

        if (adapter) {
            auto* service = (m_proxy.objectPath() == adapter->objectPath() ? adapter->service() : nullptr);
            if (service) {
                auto provider = qobject_cast<InterfaceType *>(service);
                if (provider != m_provider) {
                    m_provider = provider;
                    m_adapter = adapter;
                    if (m_provider) {
                        qCDebug(LogIpc) << "Local server found for " << m_proxy.objectPath();
                        m_proxy.refreshProvider();
                    }
                }
            }
        } else {
            if (m_adapter != nullptr) {
                m_provider = nullptr;
                m_adapter = nullptr;
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
};

}
