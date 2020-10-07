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

#if defined(FaceliftIPCLibDBus_LIBRARY)
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_IMPORT
#endif

#include <QDBusVirtualObject>
#include <QtDBus>
#include "IPCServiceAdapterBase.h"
#include "DBusIPCMessage.h"
#include "DBusIPCCommon.h"
#include "ipc-common.h"
#include "FaceliftUtils.h"
#include "DBusManagerInterface.h"
#include "FaceliftDBusMarshaller.h"

namespace facelift {

namespace dbus {

class FaceliftIPCLibDBus_EXPORT IPCDBusServiceAdapterBase : public IPCServiceAdapterBase
{
    Q_OBJECT

    typedef const char *MemberIDType;

public:

    template<typename Type>
    using IPCAdapterType = typename Type::IPCDBusAdapterType;

    class DBusVirtualObject : public QDBusVirtualObject
    {

    public:
        DBusVirtualObject(IPCDBusServiceAdapterBase &adapter) : QDBusVirtualObject(nullptr), m_adapter(adapter)
        {
        }

        QString introspect(const QString &path) const override
        {
            return m_adapter.introspect(path);
        }

        bool handleMessage(const QDBusMessage &message, const QDBusConnection &connection) override
        {
            Q_UNUSED(connection);
            return m_adapter.handleMessage(message);
        }

    private:
        IPCDBusServiceAdapterBase &m_adapter;
    };

    IPCDBusServiceAdapterBase(DBusManagerInterface& dbusManager, QObject *parent = nullptr);

    ~IPCDBusServiceAdapterBase();

    bool handleMessage(const QDBusMessage &dbusMsg);

    inline void sendPropertiesChanged(const QVariantMap& changedProperties);

    template<typename ... Args>
    void sendSignal(const QString& signalName, Args && ... args);

    template<typename ReturnType>
    void sendAsyncCallAnswer(DBusIPCMessage &replyMessage, const ReturnType returnValue);

    void sendAsyncCallAnswer(DBusIPCMessage &replyMessage);

    void send(DBusIPCMessage &message);

    virtual IPCHandlingResult handleMethodCallMessage(DBusIPCMessage &requestMessage, DBusIPCMessage &replyMessage) = 0;

    virtual QVariantMap marshalProperties() = 0;

    virtual QVariant marshalProperty(const QString& propertyName) = 0;

    virtual void setProperty(const QString& propertyName, const QVariant& value) = 0;

    void registerService() override;

    void unregisterService() override;

    virtual void appendDBUSIntrospectionData(QTextStream &s) const = 0;

    QString introspect(const QString &path) const;

    template<typename T>
    MemberIDType memberID(T member, MemberIDType memberName) const
    {
        // DBus member IDs are strings
        Q_UNUSED(member);
        return memberName;
    }

    template<typename T>
    T castFromQVariant(const QVariant& value) {
        static std::once_flag registerFlag;
        std::call_once(registerFlag, [](){registerDBusType(HelperType<T>());});
        return castFromQVariantSpecialized(HelperType<T>(), value);
    }

    template<typename T>
    QVariant castToQVariant(const T& value) {
        static std::once_flag registerFlag;
        std::call_once(registerFlag, [](){registerDBusType(HelperType<T>());});
        return castToQVariantSpecialized(HelperType<T>(), value);
    }

protected:
    DBusVirtualObject m_dbusVirtualObject;

    QString m_introspectionData;

    bool m_previousReadyState = false;
    bool m_signalsConnected = false;
    bool m_alreadyInitialized = false;

    DBusManagerInterface& m_dbusManager;
    QDBusConnection m_connection;
    QString m_address;
private:
    template<typename T, typename std::enable_if_t<!std::is_enum<T>::value, int> = 0>
    T castFromQVariantSpecialized(HelperType<T>, const QVariant& value) {
        return qdbus_cast<T>(value);
    }

    QList<QString> castFromQVariantSpecialized(HelperType<QList<QString>>, const QVariant& value) {
        return qdbus_cast<QStringList>(value); // workaround to use QList<QString> since its signature matches the QStringList
    }

    template<typename T, typename std::enable_if_t<std::is_enum<T>::value, int> = 0>
    T castFromQVariantSpecialized(HelperType<T>, const QVariant& value) {
        return static_cast<T>(qdbus_cast<int>(value));
    }

    template<typename T, typename std::enable_if_t<std::is_enum<T>::value, int> = 0>
    QList<T> castFromQVariantSpecialized(HelperType<QList<T>>, const QVariant& value) {
        QList<int> tmp = qdbus_cast<QList<int>>(value);
        QList<T> ret;
        std::transform(tmp.begin(), tmp.end(), std::back_inserter(ret), [](const int entry){return static_cast<T>(entry);});
        return ret;
    }

    template<typename T, typename std::enable_if_t<std::is_enum<T>::value, int> = 0>
    QMap<QString, T> castFromQVariantSpecialized(HelperType<QMap<QString, T>>, const QVariant& value) {
        QMap<QString, T> ret;
        QMap<QString, int> tmp = qdbus_cast<QMap<QString, int>>(value);
        for (const QString& key: tmp.keys()) {
            ret[key] = static_cast<T>(tmp[key]);
        }
        return ret;
    }

    template<typename T, typename std::enable_if_t<!std::is_convertible<T, facelift::InterfaceBase*>::value && !std::is_enum<T>::value, int> = 0>
    QVariant castToQVariantSpecialized(HelperType<T>, const T& value) {
        return QVariant::fromValue(value);
    }

    template<typename T, typename std::enable_if_t<!std::is_convertible<T, facelift::InterfaceBase*>::value && std::is_enum<T>::value, int> = 0>
    QVariant castToQVariantSpecialized(HelperType<T>, const T& value) {
        return QVariant::fromValue(static_cast<int>(value));
    }

    QVariant castToQVariantSpecialized(HelperType<QList<QString>>, const QList<QString>& value) {
        return QVariant::fromValue(QStringList(value)); // workaround to use QList<QString> since its signature matches the QStringList
    }

    template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::InterfaceBase*>::value, int> = 0>
    QVariant castToQVariantSpecialized(HelperType<T>, const T& value) {
        QString objectPath;
        if (value != nullptr) {
            objectPath = getOrCreateAdapter<typename std::remove_pointer<T>::type::IPCDBusAdapterType>(value)->objectPath();
        }
        return QVariant::fromValue(objectPath);
    }

    template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::InterfaceBase*>::value, int> = 0>
    QVariant castToQVariantSpecialized(HelperType<QList<T>>, const QList<T>& value) {
        QStringList objectPathes;
        for (T service: value) {
            objectPathes.append(getOrCreateAdapter<typename std::remove_pointer<T>::type::IPCDBusAdapterType>(service)->objectPath());
        }
        return QVariant::fromValue(objectPathes);
    }

    template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::InterfaceBase*>::value, int> = 0>
    QVariant castToQVariantSpecialized(HelperType<QMap<QString, T>>, const QMap<QString, T>& value) {
        QMap<QString, QString> objectPathesMap;
        for (const QString& key: value.keys()) {
            objectPathesMap[key] = getOrCreateAdapter<typename std::remove_pointer<T>::type::IPCDBusAdapterType>(value[key])->objectPath();
        }
        return QVariant::fromValue(objectPathesMap);
    }

    template<typename T, typename std::enable_if_t<std::is_enum<T>::value, int> = 0>
    QVariant castToQVariantSpecialized(HelperType<QList<T>>, const QList<T>& value) {
        QList<int> ret;
        std::transform(value.begin(), value.end(), std::back_inserter(ret), [](const T& entry){return static_cast<int>(entry);});
        return QVariant::fromValue(ret);
    }

    template<typename T, typename std::enable_if_t<std::is_enum<T>::value, int> = 0>
    QVariant castToQVariantSpecialized(HelperType<QMap<QString, T>>, const QMap<QString, T>& value) {
        QMap<QString, int> ret;
        for (const QString& key: value.keys()) {
            ret[key] = static_cast<int>(value[key]);
        }
        return QVariant::fromValue(ret);
    }
};

inline void IPCDBusServiceAdapterBase::sendPropertiesChanged(const QVariantMap &changedProperties)
{
    DBusIPCMessage reply(objectPath(), DBusIPCCommon::PROPERTIES_INTERFACE_NAME, DBusIPCCommon::PROPERTIES_CHANGED_SIGNAL_NAME);
    reply << interfaceName();
    QMap<QString, QDBusVariant> convertedToDBusVariant;
    for (const QString& key: changedProperties.keys()) {
        convertedToDBusVariant[key] = QDBusVariant(changedProperties[key]);
    }
    reply << QVariant::fromValue(convertedToDBusVariant);
    this->send(reply);
}

template<typename ... Args>
inline void IPCDBusServiceAdapterBase::sendSignal(const QString& signalName, Args && ... args)
{
    DBusIPCMessage signal(objectPath(), interfaceName(), signalName);
    using expander = int[];
        (void)expander{0,
            (void(signal << castToQVariant(std::forward<Args>(args))), 0)...
        };
    this->send(signal);
}

template<typename ReturnType>
inline void IPCDBusServiceAdapterBase::sendAsyncCallAnswer(DBusIPCMessage &replyMessage, const ReturnType returnValue)
{
    replyMessage << castToQVariant(returnValue);
    send(replyMessage);
}

} // end namespace dbus
} // end namespace facelift
