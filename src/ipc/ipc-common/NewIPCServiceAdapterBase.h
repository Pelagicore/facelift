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

#include "ipc-common.h"
#include "QMLAdapter.h"
#include "StringConversionHandler.h"
#include "span.h"
#include "InterfaceManagerInterface.h"

#if defined(FaceliftIPCCommonLib_LIBRARY)
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_IMPORT
#endif

namespace facelift {


class FaceliftIPCCommonLib_EXPORT NewIPCServiceAdapterBase : public QObject
{
    Q_OBJECT

public:
    Q_PROPERTY(QObject * service READ service WRITE checkedSetService)
    Q_PROPERTY(QString objectPath READ objectPath WRITE setObjectPath)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)
    Q_PROPERTY(QString address READ address WRITE setAddress)
    Q_PROPERTY(QString serviceName READ serviceName WRITE setServiceName)

    NewIPCServiceAdapterBase(InterfaceManagerInterface& interfaceManager, QObject *parent);

    ~NewIPCServiceAdapterBase();

    bool enabled() const
    {
        return m_enabled;
    }

    const QString& address() const
    {
        return m_address;
    }

    const QString& serviceName() const
    {
        return m_serviceName;
    }

    void setAddress(const QString& address);

    void setServiceName(const QString& serviceName);

    void setEnabled(bool enabled);

    void checkedSetService(QObject *service);

    const QString &objectPath() const
    {
        return m_objectPath;
    }

    void setObjectPath(const QString &objectPath);

    virtual InterfaceBase *service() const = 0;

    bool isReady() const;

    void onProviderCompleted();

    void registerService();

    void unregisterService();

    virtual void createAdapters() = 0;
    virtual void destroyAdapters() = 0;

protected:

    void setServiceAdapters(facelift::span<IPCServiceAdapterBase*> adapters)
    {
        m_ipcServiceAdapters = adapters;
    }

    template<typename ServiceType>
    ServiceType *bindToProvider(QObject *s)
    {
        auto service = qobject_cast<ServiceType *>(s);
        if (service == nullptr) {
            auto *qmlAdapter = qobject_cast<QMLAdapterBase *>(s);
            if (qmlAdapter != nullptr) {
                service = qobject_cast<ServiceType *>(qmlAdapter->providerPrivate());
            }
        }
        if (service != nullptr) {
            bindToProvider(service);
        } else {
            qFatal("Bad service type : '%s'", qPrintable(facelift::toString(s->objectName())));
        }
        return service;
    }

    template<typename ServiceType>
    void bindToProvider(ServiceType *service)
    {
        if (service->isComponentCompleted()) {
            onProviderCompleted();
        } else {
            QObject::connect(service, &InterfaceBase::componentCompleted, this, &NewIPCServiceAdapterBase::onProviderCompleted);
        }
    }

    void registerLocalService();

    void unregisterLocalService();

    virtual void setService(QObject *service) = 0;

private:

    void onValueChanged();

    facelift::span<IPCServiceAdapterBase*> m_ipcServiceAdapters;

    QString m_objectPath;
    bool m_enabled = true;
    bool m_providerReady = false;
    bool m_registered = false;
    QString m_address;
    QString m_serviceName;
    InterfaceManagerInterface& m_interfaceManager;
};

}
