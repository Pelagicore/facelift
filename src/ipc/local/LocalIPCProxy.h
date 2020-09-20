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

#include "LocalIPCMessage.h"
#include "IPCProxyBase.h"
#include "LocalIPCProxyBase.h"

#if defined(FaceliftIPCLocalLib_LIBRARY)
#  define FaceliftIPCLocalLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCLocalLib_EXPORT Q_DECL_IMPORT
#endif

namespace facelift {

namespace local {

using namespace facelift;

template<typename InterfaceType>
class LocalIPCProxy : public IPCProxyBase<InterfaceType>, protected LocalIPCProxyBase
{

public:
    typedef const char *MemberIDType;
    using InputIPCMessage = ::facelift::local::LocalIPCMessage;
    using OutputIPCMessage = ::facelift::local::LocalIPCMessage;

    template<typename Type>
    using IPCProxyType = typename Type::IPCLocalProxyType;

    LocalIPCProxy(QObject *parent = nullptr) : IPCProxyBase<InterfaceType>(parent), LocalIPCProxyBase(m_ipcBinder), m_ipcBinder(*this)
    {
        m_ipcBinder.setInterfaceName(InterfaceType::FULLY_QUALIFIED_INTERFACE_NAME);
        m_ipcBinder.setHandler(this);

        this->initBinder(m_ipcBinder);
        this->setImplementationID("Local IPC Proxy");
    }

    template<typename T>
    MemberIDType memberID(T member, const char *memberName) const
    {
        // Local member IDs are strings. TODO : change to integer
        Q_UNUSED(member);
        return memberName;
    }

    void setServiceRegistered(bool isRegistered) override
    {
        bool oldReady = this->ready();
        m_serviceRegistered = isRegistered;
        if (this->ready() != oldReady) {
            this->readyChanged();
        }

        m_ipcBinder.setServiceAvailable(isRegistered);
    }

    LocalIPCProxyBinder *ipc()
    {
        return &m_ipcBinder;
    }

    template<typename SubInterfaceType>
    typename SubInterfaceType::IPCLocalProxyType *getOrCreateSubProxy(const QString &objectPath)
    {
        return m_ipcBinder.getOrCreateSubProxy<SubInterfaceType>(objectPath);
    }

    void connectToServer()
    {
        m_ipcBinder.connectToServer();
    }

    template<typename T>
    T castFromQVariant(const QVariant& value) {
        return castFromQVariantSpecialized(HelperType<T>(), value);
    }

private:
    template<typename T> struct HelperType { };
    template<typename T, typename std::enable_if_t<!std::is_convertible<T, facelift::InterfaceBase*>::value, int> = 0>
    T castFromQVariantSpecialized(HelperType<T>, const QVariant& value) {
        return qvariant_cast<T>(value);
    }

    template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::InterfaceBase*>::value, int> = 0>
    T castFromQVariantSpecialized(HelperType<T>, const QVariant& value) {
        return getOrCreateSubProxy<typename std::remove_pointer<T>::type::IPCLocalProxyType>(qvariant_cast<QString>(value));
    }

    template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::InterfaceBase*>::value, int> = 0>
    QMap<QString, T> castFromQVariantSpecialized(HelperType<QMap<QString, T>>, const QVariant& value) {
        QMap<QString, T> ret;
        auto objectPaths = qvariant_cast<QMap<QString, QString>>(value);
        for (const QString& key: objectPaths.keys()) {
            ret[key] = getOrCreateSubProxy<typename std::remove_pointer<T>::type::IPCLocalProxyType>(objectPaths[key]);
        }
        return ret;
    }

    template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::InterfaceBase*>::value, int> = 0>
    QList<T> castFromQVariantSpecialized(HelperType<QList<T>>, const QVariant& value) {
        QList<T> ret;
        auto objectPaths = qvariant_cast<QStringList>(value);
        for (const QString& objectPath: objectPaths) {
            ret.append(getOrCreateSubProxy<typename std::remove_pointer<T>::type::IPCLocalProxyType>(objectPath));
        }
        return ret;
    }

    LocalIPCProxyBinder m_ipcBinder;
};

}

}
