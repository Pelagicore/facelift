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

#include "IPCProxyBase.h"
#include "DBusRequestHandler.h"
#include "DBusIPCProxyBinder.h"
#include "DBusManager.h"

#if defined(FaceliftIPCLibDBus_LIBRARY)
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_IMPORT
#endif

namespace facelift {

namespace dbus {

using namespace facelift;

template<typename InterfaceType>
class DBusIPCProxy : public IPCProxyBase<InterfaceType>, protected DBusRequestHandler
{

public:
    using MemberIDType = const char *;
    using InputIPCMessage = ::facelift::dbus::DBusIPCMessage;
    using OutputIPCMessage = ::facelift::dbus::DBusIPCMessage;

    template<typename Type>
    using IPCProxyType = typename Type::IPCDBusProxyType;

    DBusIPCProxy(QObject *parent = nullptr) :
        IPCProxyBase<InterfaceType>(parent),
        m_ipcBinder(DBusManager::instance(), *this)
    {
        m_ipcBinder.setInterfaceName(InterfaceType::FULLY_QUALIFIED_INTERFACE_NAME);
        m_ipcBinder.setHandler(this);

        this->initBinder(m_ipcBinder);
        this->setImplementationID("DBus IPC Proxy");
    }

    template<typename T>
    MemberIDType memberID(T member, const char *memberName) const
    {
        // DBus member IDs are strings
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

    DBusIPCProxyBinder *ipc()
    {
        return &m_ipcBinder;
    }

    template<typename SubInterfaceType>
    typename SubInterfaceType::IPCDBusProxyType *getOrCreateSubProxy(const QString &objectPath)
    {
        return m_ipcBinder.getOrCreateSubProxy<SubInterfaceType>(objectPath);
    }

    void connectToServer()
    {
        m_ipcBinder.connectToServer();
    }

    template<typename T>
    T castFromQVariant(const QVariant& value) {
        return m_ipcBinder.castFromQVariant<T>(value);
    }

protected:
    bool m_serviceRegistered = false;
private:
    DBusIPCProxyBinder m_ipcBinder;

};

} // end namespace dbus
} // end namespace facelift
