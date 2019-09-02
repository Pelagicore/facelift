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

#include "ipc-common.h"
#include "InterfaceManager.h"
#include "QMLAdapter.h"
#include "FaceliftStringConversion.h"

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

    NewIPCServiceAdapterBase(QObject *parent) : QObject(parent)
    {
    }

    virtual void registerService() = 0;

    virtual void unregisterService() = 0;

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
            if (service->isComponentCompleted()) {
                onProviderCompleted();
            } else {
                QObject::connect(service, &InterfaceBase::componentCompleted, this, &NewIPCServiceAdapterBase::onProviderCompleted);
            }
        } else {
            qFatal("Bad service type : '%s'", qPrintable(facelift::toString(s->objectName())));
        }
        return service;
    }

    bool enabled() const
    {
        return m_enabled;
    }

    void setEnabled(bool enabled)
    {
        m_enabled = enabled;
        onValueChanged();
    }

    void registerLocalService()
    {
        InterfaceManager::instance().registerAdapter(objectPath(), this);
    }

    void unregisterLocalService()
    {
        InterfaceManager::instance().unregisterAdapter(this);
    }

    virtual void setService(QObject *service) = 0;

    void checkedSetService(QObject *service)
    {
        setService(service);
        onValueChanged();
    }

    const QString &objectPath() const
    {
        return m_objectPath;
    }

    void setObjectPath(const QString &objectPath)
    {
        m_objectPath = objectPath;
        onValueChanged();
    }

    virtual InterfaceBase *service() const = 0;

    bool isReady() const
    {
        return (enabled() && m_providerReady && !objectPath().isEmpty() && (service() != nullptr));
    }

    void onValueChanged()
    {
        if (isReady()) {
            if (!m_registered) {
                registerService();
                m_registered = true;
            }
        } else {
            if (m_registered) {
                unregisterService();
                m_registered = false;
            }
        }
    }

    void onProviderCompleted()
    {
        // The parsing of the provider is finished => all our properties are set and we are ready to register our service
        m_providerReady = true;
        onValueChanged();
    }

private:
    QString m_objectPath;
    bool m_enabled = true;
    bool m_providerReady = false;
    bool m_registered = false;
};


}
