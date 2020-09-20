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
#include "IPCServiceAdapterBase.h"

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
    DBusIPCMessage sendMethodCall(const char *methodName, Args && ... args);

    template<typename ReturnType, typename ... Args>
    void sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<ReturnType> answer, Args && ... args);

    template<typename ... Args>
    void sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<void> answer, Args && ... args);

    template<typename ... Args>
    QList<QVariant> sendMethodCallWithReturn(const char *methodName, Args && ... args);

    void setHandler(DBusRequestHandler *handler);

    template<typename T>
    T castFromVariant(const QVariant& value) {
        return castFromVariantSpecialized(HelperType<T>(), value);
    }

    template<typename T>
    T castFromDBusVariant(const QVariant& value) {
        return castFromVariantSpecialized(HelperType<T>(), value);
    }

    template<typename T>
    QVariant castToVariant(const T& value) {
        return castToVariantSpecialized(HelperType<T>(), value);
    }

    template<typename T>
    QDBusVariant castToDBusVariant(const T& value) {
        return QDBusVariant(castToVariant<T>(value));
    }

private:
    void checkRegistry();

    template<typename T> struct HelperType { };

    template<typename T, typename std::enable_if_t<!std::is_convertible<T, facelift::InterfaceBase*>::value && !std::is_enum<T>::value, int> = 0>
    T castFromVariantSpecialized(HelperType<T>, const QVariant& value) {
        return qdbus_cast<T>(value);
    }

    template<typename T, typename std::enable_if_t<!std::is_convertible<T, facelift::InterfaceBase*>::value && std::is_enum<T>::value, int> = 0>
    T castFromVariantSpecialized(HelperType<T>, const QVariant& value) {
        return static_cast<T>(qdbus_cast<int>(value));
    }

    QList<QString> castFromVariantSpecialized(HelperType<QList<QString>>, const QVariant& value) {
        return qdbus_cast<QStringList>(value); // workaround to use QList<QString> since its signature matches the QStringList
    }

    template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::InterfaceBase*>::value, int> = 0>
    T castFromVariantSpecialized(HelperType<T>, const QVariant& value) {
        return getOrCreateSubProxy<typename std::remove_pointer<T>::type::IPCDBusProxyType>(qdbus_cast<DBusObjectPath>(value));
    }

    template<typename T>
    QMap<QString, T*> castFromVariantSpecialized(HelperType<QMap<QString, T*>>, const QVariant& value) {
        QMap<QString, T*> ret;
        auto objectPaths = qdbus_cast<QMap<QString, DBusObjectPath>>(value);
        for (const QString& key: objectPaths.keys()) {
            ret[key] = getOrCreateSubProxy<typename T::IPCDBusProxyType>(objectPaths[key]);
        }
        return ret;
    }

    template<typename T>
    QList<T*> castFromVariantSpecialized(HelperType<QList<T*>>, const QVariant& value) {
        QList<T*> ret;
        auto objectPaths = qdbus_cast<QStringList/*QList<DBusObjectPath>*/>(value);
        for (const DBusObjectPath& objectPath: objectPaths) {
            ret.append(getOrCreateSubProxy<typename T::IPCDBusProxyType>(objectPath));
        }
        return ret;
    }

    template<typename T, typename std::enable_if_t<!std::is_convertible<T, facelift::InterfaceBase*>::value && !std::is_enum<T>::value, int> = 0>
    QVariant castToVariantSpecialized(HelperType<T>, const T& value) {
        return QVariant::fromValue(value);
    }

    QVariant castToVariantSpecialized(HelperType<QList<QString>>, const QList<QString>& value) {
        return QVariant::fromValue(QStringList(value)); // workaround to use QList<QString> since its signature matches the QStringList
    }

    template<typename T, typename std::enable_if_t<!std::is_convertible<T, facelift::InterfaceBase*>::value && std::is_enum<T>::value, int> = 0>
    QVariant castToVariantSpecialized(HelperType<T>, const T& value) {
        return QVariant::fromValue(static_cast<int>(value));
    }

    template<typename T, typename std::enable_if_t<std::is_enum<T>::value, int> = 0>
    QVariant castToVariantSpecialized(HelperType<QList<T>>, const QList<T>& value) {
        return QVariant::fromValue(static_cast<QList<int>>(value));
    }

    template<typename T, typename std::enable_if_t<std::is_enum<T>::value, int> = 0>
    QVariant castToVariantSpecialized(HelperType<QMap<QString, T>>, const QMap<QString, T>& value) {
        QMap<QString, int> ret;
        for (const QString& key: value.keys()) {
            ret[key] = static_cast<int>(value[key]);
        }
        return QVariant::fromValue(ret);
    }

    QString m_interfaceName;
    QDBusServiceWatcher m_busWatcher;
    DBusRequestHandler *m_serviceObject = nullptr;
    bool m_serviceAvailable = false;
    DBusManagerInterface &m_dbusManager;
};


template<typename ... Args>
inline DBusIPCMessage DBusIPCProxyBinder::sendMethodCall(const char *methodName, Args && ... args)
{
    DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
    using expander = int[];
        (void)expander{0,
            (void(msg << castToVariant(std::forward<Args>(args))), 0)...
        };
    auto replyMessage = this->call(msg);
    if (replyMessage.isErrorMessage()) {
        onServerNotAvailableError(methodName);
    }
    return replyMessage;
}

template<typename ReturnType, typename ... Args>
inline void DBusIPCProxyBinder::sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<ReturnType> answer, Args && ... args)
{
    DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
    using expander = int[];
        (void)expander{0,
            (void(msg << castToVariant(std::forward<Args>(args))), 0)...
        };
    asyncCall(msg, this, [this, answer](DBusIPCMessage &msg) {
        ReturnType returnValue;
        if (msg.isReplyMessage()) {
            if (!msg.arguments().isEmpty()) {
                returnValue = castFromVariant<ReturnType>(msg.arguments()[0]);
            }
            answer(returnValue);
        } else {
            qCWarning(LogIpc) << "Error received" << msg.toString();
        }
    });
}

template<typename ... Args>
inline void DBusIPCProxyBinder::sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<void> answer, Args && ... args)
{
    DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
    using expander = int[];
        (void)expander{0,
            (void(msg << castToVariant(std::forward<Args>(args))), 0)...
        };
    asyncCall(msg, this, [answer](DBusIPCMessage &msg) {
        Q_UNUSED(msg)
        answer();
    });
}

template<typename ... Args>
inline QList<QVariant> DBusIPCProxyBinder::sendMethodCallWithReturn(const char *methodName, Args && ... args)
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
    msg << castToVariant(property);
    msg << QVariant::fromValue(castToDBusVariant(value));
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
