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
#include "IPCProxyBinderBase.h"


#if defined(FaceliftIPCCommonLib_LIBRARY)
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_IMPORT
#endif

namespace facelift {

template<typename AdapterType>
class IPCProxyBase : public AdapterType
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

protected:
    bool m_serviceReady = false;
    IPCProxyBinderBase *m_ipcBinder = nullptr;

};

}
