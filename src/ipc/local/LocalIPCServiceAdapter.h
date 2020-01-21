/**********************************************************************
**
** Copyright (C) 2019 Luxoft Sweden AB
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

#include "LocalIPC.h"
#include "IPCServiceAdapterBase.h"

#if defined(FaceliftIPCLocalLib_LIBRARY)
#  define FaceliftIPCLocalLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCLocalLib_EXPORT Q_DECL_IMPORT
#endif

namespace facelift {

namespace local {

using namespace facelift;

class FaceliftIPCLocalLib_EXPORT LocalIPCServiceAdapterBase : public IPCServiceAdapterBase
{
    Q_OBJECT

    typedef const char *MemberIDType;

public:
    template<typename Type>
    using IPCAdapterType = typename Type::IPCLocalAdapterType;

    LocalIPCServiceAdapterBase(QObject *parent = nullptr);

    ~LocalIPCServiceAdapterBase();

    IPCHandlingResult handleMessage(LocalIPCMessage &message);

    void flush();

    template<typename Type>
    void serializeValue(LocalIPCMessage &msg, const Type &v);

    template<typename Type>
    void deserializeValue(LocalIPCMessage &msg, Type &v);

    void initOutgoingSignalMessage();

    template<typename MemberID, typename ... Args>
    void sendSignal(MemberID signalID, const Args & ... args);

    template<typename ReturnType>
    void sendAsyncCallAnswer(LocalIPCMessage &replyMessage, const ReturnType returnValue);

    void sendAsyncCallAnswer(LocalIPCMessage &replyMessage);

    virtual IPCHandlingResult handleMethodCallMessage(LocalIPCMessage &requestMessage, LocalIPCMessage &replyMessage) = 0;

    virtual void serializePropertyValues(LocalIPCMessage &msg, bool isCompleteSnapshot);

    void registerService() override;

    void unregisterService() override;

    Q_SIGNAL void messageSent(LocalIPCMessage &message);

    void send(LocalIPCMessage &message)
    {
        emit messageSent(message);
    }

    void sendReply(LocalIPCMessage &message)
    {
        Q_UNUSED(message);
        Q_ASSERT(false);
    }

    template<typename Type>
    void serializeOptionalValue(LocalIPCMessage &msg, const Type &currentValue, Type &previousValue, bool isCompleteSnapshot);

    template<typename Type>
    void serializeOptionalValue(LocalIPCMessage &msg, const Type &currentValue, bool isCompleteSnapshot);

    virtual void appendDBUSIntrospectionData(QTextStream &s) const = 0;

    QString introspect(const QString &path) const;

    template<typename T>
    MemberIDType memberID(T member, MemberIDType memberName) const
    {
        // Local member IDs are strings
        Q_UNUSED(member);
        return memberName;
    }

protected:
    std::unique_ptr<LocalIPCMessage> m_pendingOutgoingMessage;

    QString m_introspectionData;
    QString m_serviceName;

    bool m_alreadyInitialized = false;
};


template<typename ServiceType>
class LocalIPCServiceAdapter : public LocalIPCServiceAdapterBase
{
    using LocalIPCServiceAdapterBase::registerService;

public:
    typedef ServiceType TheServiceType;
    using InputIPCMessage = ::facelift::local::LocalIPCMessage;
    using OutputIPCMessage = ::facelift::local::LocalIPCMessage;

    LocalIPCServiceAdapter(QObject *parent) : LocalIPCServiceAdapterBase(parent)
    {
        setInterfaceName(ServiceType::FULLY_QUALIFIED_INTERFACE_NAME);
    }

    ~LocalIPCServiceAdapter()
    {
        unregisterService();
    }

    void registerService(const QString &objectPath, ServiceType *service)
    {
        setObjectPath(objectPath);
        m_service = service;
        this->registerService();
    }

    void unregisterService() override
    {
        LocalIPCServiceAdapterBase::unregisterService();
        setObjectPath("");
        m_service = nullptr;
    }

    ServiceType *service() const override
    {
        return m_service;
    }

    void registerService(const QString &objectPath, InterfaceBase *serverObject) override
    {
        Q_ASSERT(qobject_cast<ServiceType *>(serverObject) != nullptr);
        registerService(objectPath, static_cast<ServiceType *>(serverObject));  // TODO: get rid of that cast
    }

protected:
    QPointer<ServiceType> m_service;
};

}

}
