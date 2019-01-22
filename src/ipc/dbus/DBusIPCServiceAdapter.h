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

#include "ipc-dbus.h"
#include "ipc-common/IPCServiceAdapterBase.h"


#if defined(FaceliftIPCLibDBus_LIBRARY)
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_IMPORT
#endif

namespace facelift {

namespace dbus {

using namespace facelift;

class FaceliftIPCLibDBus_EXPORT DBusIPCServiceAdapterBase : public IPCServiceAdapterBase
{
    Q_OBJECT

    typedef const char *MemberIDType;

public:

    class DBusVirtualObject : public QDBusVirtualObject
    {

    public:
        DBusVirtualObject(DBusIPCServiceAdapterBase &adapter) : QDBusVirtualObject(nullptr), m_adapter(adapter)
        {
        }

        QString introspect(const QString &path) const override
        {
            return m_adapter.introspect(path);
        }

        bool handleMessage(const QDBusMessage &message, const QDBusConnection &connection) override
        {
            return m_adapter.handleMessage(message, connection);
        }

    private:
        DBusIPCServiceAdapterBase &m_adapter;
    };

    DBusIPCServiceAdapterBase(QObject *parent = nullptr);

    ~DBusIPCServiceAdapterBase();

    bool handleMessage(const QDBusMessage &dbusMsg, const QDBusConnection &connection);

    void flush();

    template<typename Type>
    void serializeValue(DBusIPCMessage &msg, const Type &v);

    template<typename Type>
    void deserializeValue(DBusIPCMessage &msg, Type &v);

    void initOutgoingSignalMessage();

    template<typename MemberID, typename ... Args>
    void sendSignal(MemberID signalID, const Args & ... args);

    template<typename ReturnType>
    void sendAsyncCallAnswer(DBusIPCMessage &replyMessage, const ReturnType returnValue);

    void sendAsyncCallAnswer(DBusIPCMessage &replyMessage);

    virtual IPCHandlingResult handleMethodCallMessage(DBusIPCMessage &requestMessage, DBusIPCMessage &replyMessage) = 0;

    virtual void serializePropertyValues(DBusIPCMessage &msg, bool isCompleteSnapshot);

    void init();

    DBusManager &dbusManager()
    {
        return DBusManager::instance();
    }

    template<typename Type>
    void serializeOptionalValue(DBusIPCMessage &msg, const Type &currentValue, Type &previousValue, bool isCompleteSnapshot);

    template<typename Type>
    void serializeOptionalValue(DBusIPCMessage &msg, const Type &currentValue, bool isCompleteSnapshot);

    virtual void appendDBUSIntrospectionData(QTextStream &s) const = 0;

    QString introspect(const QString &path) const;

    template<typename T>
    MemberIDType memberID(T member, MemberIDType memberName) const
    {
        // DBus member IDs are strings
        Q_UNUSED(member);
        return memberName;
    }

protected:
    std::unique_ptr<DBusIPCMessage> m_pendingOutgoingMessage;
    DBusVirtualObject m_dbusVirtualObject;

    QString m_introspectionData;
    QString m_serviceName;

    bool m_alreadyInitialized = false;
};


template<typename ServiceType>
class DBusIPCServiceAdapter : public DBusIPCServiceAdapterBase
{
public:
    typedef ServiceType TheServiceType;

    DBusIPCServiceAdapter(QObject *parent) : DBusIPCServiceAdapterBase(parent)
    {
        setInterfaceName(ServiceType::FULLY_QUALIFIED_INTERFACE_NAME);
    }

    ServiceType *service() const override
    {
        return m_service;
    }

    void setService(ServiceType *service)
    {
        m_service = service;
    }

protected:
    QPointer<ServiceType> m_service;
};

}

}
