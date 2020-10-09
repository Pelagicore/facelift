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
#include "DBusIPCCommon.h"
#include "FaceliftUtils.h"
#include "FaceliftDBusMarshaller.h"
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
    DBusIPCMessage sendMethodCall(const char *methodName, Args && ... args) const;

    template<typename ReturnType, typename ... Args>
    void sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<ReturnType> answer, Args && ... args);

    template<typename ... Args>
    void sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<void> answer, Args && ... args);

    template<typename ... Args>
    QList<QVariant> sendMethodCallWithReturn(const char *methodName, Args && ... args) const;

    void setHandler(DBusRequestHandler *handler);

    template<typename T>
    T castFromQVariant(const QVariant& value) {
        static std::once_flag registerFlag;
        std::call_once(registerFlag, [](){registerDBusType(HelperType<T>());});
        return castFromQVariantSpecialized(HelperType<T>(), value);
    }

    template<typename T>
    QVariant castToQVariant(const T& value) const {
        static std::once_flag registerFlag;
        std::call_once(registerFlag, [](){registerDBusType(HelperType<T>());});
        return castToQVariantSpecialized(HelperType<T>(), value);
    }

private:
    void checkRegistry();

    template<typename T, typename std::enable_if_t<!std::is_convertible<T, facelift::InterfaceBase*>::value && !std::is_enum<T>::value, int> = 0>
    T castFromQVariantSpecialized(HelperType<T>, const QVariant& value) {
        return qdbus_cast<T>(value);
    }

    QList<QString> castFromQVariantSpecialized(HelperType<QList<QString>>, const QVariant& value) {
        return qdbus_cast<QStringList>(value); // workaround to use QList<QString> since its signature matches the QStringList
    }

    template<typename T, typename std::enable_if_t<!std::is_convertible<T, facelift::InterfaceBase*>::value && std::is_enum<T>::value, int> = 0>
    T castFromQVariantSpecialized(HelperType<T>, const QVariant& value) {
        return static_cast<T>(qdbus_cast<int>(value));
    }

    template<typename T, typename std::enable_if_t<!std::is_convertible<T, facelift::InterfaceBase*>::value && std::is_enum<T>::value, int> = 0>
    QList<T> castFromQVariantSpecialized(HelperType<QList<T>>, const QVariant& value) {
        QList<T> ret;
        QList<int> tmp = qdbus_cast<QList<int>>(value);
        std::transform(tmp.begin(), tmp.end(), std::back_inserter(ret), [](const int entry){return static_cast<T>(entry);});
        return ret;
    }

    template<typename T, typename std::enable_if_t<!std::is_convertible<T, facelift::InterfaceBase*>::value && std::is_enum<T>::value, int> = 0>
    QMap<QString, T> castFromQVariantSpecialized(HelperType<QMap<QString, T>>, const QVariant& value) {
        QMap<QString, T> ret;
        QMap<QString, int> tmp = qdbus_cast<QMap<QString, int>>(value);
        for (const QString& key: tmp.keys()) {
            ret[key] = static_cast<T>(tmp[key]);
        }
        return ret;
    }

    template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::InterfaceBase*>::value, int> = 0>
    T castFromQVariantSpecialized(HelperType<T>, const QVariant& value) {
        return getOrCreateSubProxy<typename std::remove_pointer<T>::type::IPCDBusProxyType>(qdbus_cast<QString>(value));
    }

    template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::InterfaceBase*>::value, int> = 0>
    QMap<QString, T> castFromQVariantSpecialized(HelperType<QMap<QString, T>>, const QVariant& value) {
        QMap<QString, T> ret;
        auto objectPaths = qdbus_cast<QMap<QString, QString>>(value);
        for (const QString& key: objectPaths.keys()) {
            ret[key] = getOrCreateSubProxy<typename std::remove_pointer<T>::type::IPCDBusProxyType>(objectPaths[key]);
        }
        return ret;
    }

    template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::InterfaceBase*>::value, int> = 0>
    QList<T> castFromQVariantSpecialized(HelperType<QList<T>>, const QVariant& value) {
        QList<T> ret;
        auto objectPaths = qdbus_cast<QStringList>(value);
        for (const QString& objectPath: objectPaths) {
            ret.append(getOrCreateSubProxy<typename std::remove_pointer<T>::type::IPCDBusProxyType>(objectPath));
        }
        return ret;
    }

    template<typename T, typename std::enable_if_t<!std::is_convertible<T, facelift::InterfaceBase*>::value && !std::is_enum<T>::value, int> = 0>
    QVariant castToQVariantSpecialized(HelperType<T>, const T& value) const {
        return QVariant::fromValue(value);
    }

    QVariant castToQVariantSpecialized(HelperType<QList<QString>>, const QList<QString>& value) const {
        return QVariant::fromValue(QStringList(value)); // workaround to use QList<QString> since its signature matches the QStringList
    }

    template<typename T, typename std::enable_if_t<!std::is_convertible<T, facelift::InterfaceBase*>::value && std::is_enum<T>::value, int> = 0>
    QVariant castToQVariantSpecialized(HelperType<T>, const T& value) const {
        return QVariant::fromValue(static_cast<int>(value));
    }

    template<typename T, typename std::enable_if_t<std::is_enum<T>::value, int> = 0>
    QVariant castToQVariantSpecialized(HelperType<QList<T>>, const QList<T>& value) const {
        QList<int> ret;
        std::transform(value.begin(), value.end(), std::back_inserter(ret), [](const T entry){return static_cast<int>(entry);});
        return QVariant::fromValue(ret);
    }

    template<typename T, typename std::enable_if_t<std::is_enum<T>::value, int> = 0>
    QVariant castToQVariantSpecialized(HelperType<QMap<QString, T>>, const QMap<QString, T>& value) const {
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
inline DBusIPCMessage DBusIPCProxyBinder::sendMethodCall(const char *methodName, Args && ... args) const
{
    DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
    using expander = int[];
        (void)expander{0,
            (void(msg << castToQVariant(std::forward<Args>(args))), 0)...
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
            (void(msg << castToQVariant(std::forward<Args>(args))), 0)...
        };
    asyncCall(msg, this, [this, answer](DBusIPCMessage &msg) {
        ReturnType returnValue;
        if (msg.isReplyMessage()) {
            if (!msg.arguments().isEmpty()) {
                returnValue = castFromQVariant<ReturnType>(msg.arguments().first());
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
            (void(msg << castToQVariant(std::forward<Args>(args))), 0)...
        };
    asyncCall(msg, this, [answer](DBusIPCMessage &msg) {
        Q_UNUSED(msg)
        answer();
    });
}

template<typename ... Args>
inline QList<QVariant> DBusIPCProxyBinder::sendMethodCallWithReturn(const char *methodName, Args && ... args) const
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
    DBusIPCMessage msg(m_serviceName, objectPath(), DBusIPCCommon::PROPERTIES_INTERFACE_NAME, DBusIPCCommon::SET_PROPERTY_MESSAGE_NAME);
    msg << QVariant::fromValue(m_interfaceName);
    msg << castToQVariant(property);
    msg << QVariant::fromValue(QDBusVariant(castToQVariant(value)));
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
