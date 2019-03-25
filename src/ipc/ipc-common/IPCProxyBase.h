/**********************************************************************
**
** Copyright (C) 2018 Luxoft Sweden AB
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

#if defined(FaceliftIPCLibLocal_LIBRARY)
#  define FaceliftIPCLibLocal_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCLibLocal_EXPORT Q_DECL_IMPORT
#endif

namespace facelift {


class FaceliftIPCLibLocal_EXPORT IPCProxyNewBase
{
public:
    IPCProxyNewBase(InterfaceBase &owner);

    virtual void refreshProvider() = 0;

    const QString &objectPath() const
    {
        return m_ipc.objectPath();
    }

    IPCProxyBinderBase *ipc()
    {
        return &m_ipc;
    }

private:
    facelift::IPCProxyBinderBase m_ipc;

};


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
        auto service = m_interfaceManager.serviceMatches(m_proxy.objectPath(), adapter);
        if (service) {
            m_provider = nullptr;
            m_proxy.refreshProvider();
        }
    }

    void onLocalAdapterAvailable(NewIPCServiceAdapterBase *adapter)
    {
        auto service = m_interfaceManager.serviceMatches(m_proxy.objectPath(), adapter);
        if (service) {
            auto provider = qobject_cast<InterfaceType *>(service);
            m_provider = provider;
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
    IPCProxyNewBase &m_proxy;
    InterfaceManager &m_interfaceManager = InterfaceManager::instance();
};


class FaceliftIPCLibLocal_EXPORT NotAvailableImplBase
{

protected:
    static void logMethodCall(const InterfaceBase &i, const char *methodName);
    static void logSetterCall(const InterfaceBase &i, const char *propertyName);
    static void logGetterCall(const InterfaceBase &i, const char *propertyName);
};

template<typename InterfaceType>
class NotAvailableImpl : public InterfaceType, protected NotAvailableImplBase
{
public:
    template<typename ElementType>
    struct NotAvailableModel
    {
        static Model<ElementType> &value()
        {
            static TheModel instance;
            return instance;
        }

        class TheModel : public Model<ElementType>
        {
        public:
            ElementType elementAt(int index) const override
            {
                Q_UNUSED(index);
                Q_ASSERT(false);
                return ElementType {};
            }
        };

    };

    template<typename Type>
    struct NotAvailableValue
    {
        static const Type &value()
        {
            static Type instance = {};
            return instance;
        }
    };

    template<typename Type>
    struct NotAvailableList
    {
        static const QList<Type> &value()
        {
            static QList<Type> instance;
            return instance;
        }
    };

    template<typename Type>
    void logSetterCall(const char *propertyName, const Type &value) const
    {
        Q_UNUSED(value);
        NotAvailableImplBase::logSetterCall(*this, propertyName);
    }

    void logGetterCall(const char *propertyName) const
    {
        NotAvailableImplBase::logGetterCall(*this, propertyName);
    }

    template<typename ... Args>
    void logMethodCall(const char *methodName, const Args & ... args) const
    {
        M_UNUSED(args ...);
        NotAvailableImplBase::logMethodCall(*this, methodName);
    }

    bool ready() const override
    {
        return false;
    }

};



}
