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

#if defined(FaceliftIPCLibDBus_LIBRARY)
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_IMPORT
#endif

#include <QtDBus>
#include <QDBusServiceWatcher>
#include "IPCProxyBinderBase.h"
#include "DBusIPCMessage.h"
#include "FaceliftUtils.h"
#include "DBusIPCCommon.h"
#include "DBusManagerInterface.h"

class QDBusMessage;

namespace facelift {
namespace dbus {

class DBusRequestHandler;
class DBusObjectRegistry;

class FaceliftIPCLibDBus_EXPORT DBusIPCProxyBinder : public IPCProxyBinderBase
{
    Q_OBJECT

public:

    template<typename Type>
    using IPCProxyType = typename Type::IPCDBusProxyType;

    DBusIPCProxyBinder(DBusManagerInterface& dbusManager, InterfaceBase &owner, QObject *parent = nullptr);

    const QString &interfaceName() const
    {
        return m_interfaceName;
    }

    void setObjectPath(const QString &objectPath) override;

    void setInterfaceName(const QString &name);

    Q_SLOT void onPropertiesChanged(const QDBusMessage &dbusMessage);

    Q_SLOT void handleGenericSignals(const QDBusMessage &msg);

    void bindToIPC() override;

    void onServiceNameKnown();

    void setServiceAvailable(bool isRegistered);

    bool isServiceAvailable() const override
    {
        return m_serviceAvailable;
    }

    void requestPropertyValues();

    DBusIPCMessage call(DBusIPCMessage &message) const;

    void asyncCall(DBusIPCMessage &message, const QObject *context, std::function<void(DBusIPCMessage &message)> callback);

    void onServerNotAvailableError(const QString &propertyName) const;

    template<typename PropertyType>
    void sendSetterCall(const QString& property, const PropertyType &value);

    template<typename ... Args>
    DBusIPCMessage sendMethodCall(const char *methodName, const Args & ... args) const;

    template<typename ReturnType, typename ... Args>
    void sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<ReturnType> answer, const Args & ... args);

    template<typename ... Args>
    void sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<void> answer, const Args & ... args);

    template<typename ... Args>
    QList<QVariant> sendMethodCallWithReturn(const char *methodName, const Args & ... args) const;

    void setHandler(DBusRequestHandler *handler);

private:
    void checkRegistry();

    QString m_interfaceName;
    QDBusServiceWatcher m_busWatcher;
    DBusRequestHandler *m_serviceObject = nullptr;
    bool m_serviceAvailable = false;
    DBusManagerInterface &m_dbusManager;
};


template<typename ... Args>
inline DBusIPCMessage DBusIPCProxyBinder::sendMethodCall(const char *methodName, const Args & ... args) const
{
    DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
    auto argTuple = std::make_tuple(args ...);
    for_each_in_tuple(argTuple, [&msg](const auto &v){msg << QVariant::fromValue(v);});
    auto replyMessage = this->call(msg);
    if (replyMessage.isErrorMessage()) {
        onServerNotAvailableError(methodName);
    }
    return replyMessage;
}

template<typename ReturnType, typename ... Args>
inline void DBusIPCProxyBinder::sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<ReturnType> answer, const Args & ... args)
{
    DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
    auto argTuple = std::make_tuple(args ...);
    for_each_in_tuple(argTuple, [&msg](const auto &v){msg << QVariant::fromValue(v);});
    asyncCall(msg, this, [this, answer](DBusIPCMessage &msg) {
        ReturnType returnValue;
        if (msg.isReplyMessage()) {
            returnValue = (!msg.arguments().isEmpty() ? qdbus_cast<ReturnType>(msg.arguments()[0]): ReturnType());
            answer(returnValue);
        } else {
            qCWarning(LogIpc) << "Error received" << msg.toString();
        }
    });
}

template<typename ... Args>
inline void DBusIPCProxyBinder::sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<void> answer, const Args & ... args)
{
    DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
    auto argTuple = std::make_tuple(args ...);
    for_each_in_tuple(argTuple, [&msg](const auto &v){msg << QVariant::fromValue(v);});
    asyncCall(msg, this, [answer](DBusIPCMessage &msg) {
        Q_UNUSED(msg);
        answer();
    });
}

template<typename ... Args>
inline QList<QVariant> DBusIPCProxyBinder::sendMethodCallWithReturn(const char *methodName, const Args & ... args) const
{
    DBusIPCMessage msg = sendMethodCall(methodName, args ...);
    QList<QVariant> ret;
    if (msg.isReplyMessage()) {
        ret = msg.arguments();
    }
    return ret;
}

template<typename PropertyType>
inline void DBusIPCProxyBinder::sendSetterCall(const QString &property, const PropertyType &value)
{
    DBusIPCMessage msg(m_serviceName, objectPath(), DBusIPCCommon::PROPERTIES_INTERFACE_NAME, DBusIPCCommon::SET_PROPERTY);
    msg << QVariant::fromValue(m_interfaceName);
    msg << QVariant::fromValue(property);
    msg << QVariant::fromValue(QDBusVariant(QVariant::fromValue(value)));
    if (isSynchronous()) {
        auto replyMessage = call(msg);
        if (replyMessage.isErrorMessage()) {
            onServerNotAvailableError(property);
        }
    } else {
        asyncCall(msg, this, [this, property](const DBusIPCMessage &replyMessage) {
            if (replyMessage.isErrorMessage()) {
                onServerNotAvailableError(property);
            }
        });
    }
}

} // end namespace dbus
} // end namespace facelift
