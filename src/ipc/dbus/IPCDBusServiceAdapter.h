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

#include <QDBusVirtualObject>
#include "IPCServiceAdapterBase.h"
#include "DBusIPCMessage.h"
#include "ipc-common.h"
#include "IPCDBusServiceAdapterBase.h"
#include "DBusManagerInterface.h"

#if defined(FaceliftIPCLibDBus_LIBRARY)
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_IMPORT
#endif

namespace facelift {

namespace dbus {

using namespace facelift;

template<typename ServiceType>
class IPCDBusServiceAdapter : public IPCDBusServiceAdapterBase
{
    using IPCDBusServiceAdapterBase::registerService;

public:
    typedef ServiceType TheServiceType;
    using InputIPCMessage = ::facelift::dbus::DBusIPCMessage;
    using OutputIPCMessage = ::facelift::dbus::DBusIPCMessage;

    IPCDBusServiceAdapter(DBusManagerInterface& dbusManager, QObject *parent) :
        IPCDBusServiceAdapterBase(dbusManager, parent)
    {
        setInterfaceName(ServiceType::FULLY_QUALIFIED_INTERFACE_NAME);
    }

    ~IPCDBusServiceAdapter() {
        unregisterService();
    }

    void registerService(const QString& objectPath, ServiceType *service) {
        setObjectPath(objectPath);
        m_service = service;
        this->registerService();
    }

    void unregisterService() override {
        IPCDBusServiceAdapterBase::unregisterService();
        setObjectPath("");
        m_service = nullptr;
    }

    ServiceType *service() const override
    {
        return m_service;
    }

    void registerService(const QString &objectPath, const QString& address, const QString& serviceName, InterfaceBase* serverObject) override {
        Q_ASSERT(qobject_cast<ServiceType*>(serverObject) != nullptr);
        if (address.isEmpty()) {
            m_connection = QDBusConnection::sessionBus();
        }
        else {
            m_connection = QDBusConnection::connectToBus(address, address);
        }
        if (m_connection.isConnected()) {
            m_connection.registerService(serviceName);
        }
        else {
            qCCritical(LogIpc()) << "Not connected to DBUS at address:" << (address.isEmpty() ? QStringLiteral("session bus"): address);
        }
        registerService(objectPath, static_cast<ServiceType *>(serverObject));
    }

    template<typename T>
    const char* typeToSignature() const
    {
        static std::once_flag registerFlag;
        std::call_once(registerFlag, [](){registerDBusType(HelperType<T>());});
        return typeToSignatureSpecialized(HelperType<T>());
    }
protected:

    template<typename T, typename std::enable_if_t<!std::is_convertible<T, facelift::InterfaceBase*>::value && !std::is_enum<T>::value, int> = 0>
    const char* typeToSignatureSpecialized(HelperType<T>) const
    {
        return QDBusMetaType::typeToSignature(qMetaTypeId<T>());
    }

    template<typename T, typename std::enable_if_t<!std::is_convertible<T, facelift::InterfaceBase*>::value && std::is_enum<T>::value, int> = 0>
    const char* typeToSignatureSpecialized(HelperType<T>) const
    {
        return QDBusMetaType::typeToSignature(qMetaTypeId<int>());
    }

    template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::InterfaceBase*>::value, int> = 0>
    inline const char* typeToSignatureSpecialized(HelperType<T>) const
    {
        return QDBusMetaType::typeToSignature(qMetaTypeId<QString>());
    }

    inline const char* typeToSignatureSpecialized(HelperType<QList<QString>>) const
    {
        return QDBusMetaType::typeToSignature(qMetaTypeId<QStringList>());
    }

    template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::InterfaceBase*>::value, int> = 0>
    inline const char* typeToSignatureSpecialized(HelperType<QList<T>>) const
    {
        return QDBusMetaType::typeToSignature(qMetaTypeId<QStringList>());
    }

    template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::InterfaceBase*>::value, int> = 0>
    inline const char* typeToSignatureSpecialized(HelperType<QMap<QString, T>>) const
    {
        return QDBusMetaType::typeToSignature(qMetaTypeId<QMap<QString, QString>>());
    }

    template<typename T, typename std::enable_if_t<std::is_enum<T>::value, int> = 0>
    inline const char* typeToSignatureSpecialized(HelperType<QList<T>>) const
    {
        return QDBusMetaType::typeToSignature(qMetaTypeId<QList<int>>());
    }

    template<typename T, typename std::enable_if_t<std::is_enum<T>::value, int> = 0>
    inline const char* typeToSignatureSpecialized(HelperType<QMap<QString, T>>) const
    {
        return QDBusMetaType::typeToSignature(qMetaTypeId<QMap<QString, int>>());
    }
    QPointer<ServiceType> m_service;
};

}

}
