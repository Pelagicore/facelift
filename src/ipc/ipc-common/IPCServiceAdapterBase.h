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
#include "FaceliftModel.h"


#if defined(FaceliftIPCCommonLib_LIBRARY)
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_IMPORT
#endif

namespace facelift {

class FaceliftIPCCommonLib_EXPORT IPCServiceAdapterBase : public QObject
{
    Q_OBJECT

public:
    IPCServiceAdapterBase(QObject *parent = nullptr);

    const QString &interfaceName() const
    {
        return m_interfaceName;
    }

    void setInterfaceName(const QString &name)
    {
        m_interfaceName = name;
    }

    virtual void registerService() = 0;

    virtual void connectSignals() = 0;

    virtual InterfaceBase *service() const = 0;

    const QString &objectPath() const
    {
        return m_objectPath;
    }

    void setObjectPath(const QString &objectPath)
    {
        m_objectPath = objectPath;
    }

    virtual void registerService(const QString &objectPath, const QString& address, const QString& serviceName, InterfaceBase* serverObject) = 0;

    virtual void unregisterService() = 0;

    QString generateObjectPath(const QString &parentPath) const;


    template<typename InterfaceAdapterType>
    InterfaceAdapterType *getOrCreateAdapter(typename InterfaceAdapterType::ServiceType *service)
    {
        if (service == nullptr) {
            return nullptr;
        }

        // Look for an existing adapter
        for (auto &adapter : m_subAdapters) {
            if (adapter && (adapter->service() == service)) {
                return qobject_cast<InterfaceAdapterType *>(adapter.data());
            }
        }

        auto serviceAdapter = new InterfaceAdapterType(this);  // Destroy the adapter when either the parent adapter is destroyed or when the service is destroyed
        // ensure that service adapter is removed from the list before getting deleted
        QObject::connect(service, &QObject::destroyed, this, [this, serviceAdapter]() {
            m_subAdapters.removeAll(serviceAdapter);
            serviceAdapter->deleteLater();
        });

        serviceAdapter->registerService(this->generateObjectPath(this->objectPath()), service);
        m_subAdapters.append(serviceAdapter);

        return serviceAdapter;
    }
private:
    QList<QPointer<IPCServiceAdapterBase> > m_subAdapters;
    QString m_objectPath;
    QString m_interfaceName;
};

}
