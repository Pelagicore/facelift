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
#include "IPCServiceAdapterBase.h"

namespace facelift {

namespace local {

using namespace facelift;

class LocalIPCServiceAdapterBase : public IPCServiceAdapterBase
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

    void send(LocalIPCMessage &message);

    void sendReply(LocalIPCMessage &message);

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

    bool m_previousReadyState = false;

    bool m_alreadyInitialized = false;
};


}

}
