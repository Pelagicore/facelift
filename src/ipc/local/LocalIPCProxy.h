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

    template<typename Type>
    void serializeValue(LocalIPCMessage &msg, const Type &v)
    {
        typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
        IPCTypeHandler<SerializedType>::write(msg, IPCTypeRegisterHandler<Type>::convertToSerializedType(v, *this));
    }

    template<typename Type>
    void deserializeValue(LocalIPCMessage &msg, Type &v)
    {
        typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
        SerializedType serializedValue;
        IPCTypeHandler<SerializedType>::read(msg.inputPayLoad(), serializedValue);
        IPCTypeRegisterHandler<Type>::convertToDeserializedType(v, serializedValue, *this);
    }

    template<typename Type>
    bool deserializeOptionalValue(LocalIPCMessage &msg, Type &value, bool isCompleteSnapshot)
    {
        bool b = true;
        if (!isCompleteSnapshot) {
            msg.inputPayLoad().readNextParameter(b);
        }
        if (b) {
            this->deserializeValue(msg, value);
        }
        return b;
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

    bool deserializeReadyValue(LocalIPCMessage &msg, bool isCompleteSnapshot)
    {
        bool previousIsReady = this->ready();
        deserializeOptionalValue(msg, this->m_serviceReady, isCompleteSnapshot);
        return (this->ready() != previousIsReady);
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

private:
    LocalIPCProxyBinder m_ipcBinder;
};

}

}
