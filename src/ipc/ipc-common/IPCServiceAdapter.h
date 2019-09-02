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
#include "NewIPCServiceAdapterBase.h"

#if defined(FaceliftIPCCommonLib_LIBRARY)
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_IMPORT
#endif

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

}
