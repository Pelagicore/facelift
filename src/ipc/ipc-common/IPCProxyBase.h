/**********************************************************************
**
** Copyright (C) 2020 Luxoft Sweden AB
**
** This file is part of the FaceLift project
**
** Permission is hereby granted, freIPCServiceAdapterBasee of charge, to any person
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

#include "ipc-common.h"
#include "IPCProxyBaseBase.h"
#include "IPCProxyBinderBase.h"


namespace facelift {

template<typename AdapterType>
class IPCProxyBase : public AdapterType, protected IPCProxyBaseBase
{

public:
    using InterfaceType = AdapterType;

public:
    IPCProxyBase(QObject *parent) : AdapterType(parent)
    {
    }

    template<typename BinderType>
    void initBinder(BinderType &binder)
    {
        m_ipcBinder = &binder;
        QObject::connect(this, &InterfaceBase::componentCompleted, &binder, &BinderType::onComponentCompleted);
    }

    bool isSynchronous() const
    {
        return m_ipcBinder->isSynchronous();
    }

    bool ready() const override final
    {
        auto r = m_serviceReady;
        return r;
    }

    template<typename ProxyType>
    class InterfacePropertyIPCProxyHandler
    {

    public:
        InterfacePropertyIPCProxyHandler(IPCProxyBase &owner) : m_owner(owner)
        {
        }

        void update(const QString &objectPath)
        {
            if (m_proxy && (m_proxy->ipc()->objectPath() != objectPath)) {
                m_proxy = nullptr;
            }
            if (!m_proxy) {
                m_proxy = m_owner.m_ipcBinder->template getOrCreateSubProxy<ProxyType>(objectPath);
            }
        }

        ProxyType *getValue() const
        {
            return m_proxy;
        }

    private:
        QPointer<ProxyType> m_proxy;
        IPCProxyBase &m_owner;
    };

protected:
    bool m_serviceReady = false;
    IPCProxyBinderBase *m_ipcBinder = nullptr;

};

}
