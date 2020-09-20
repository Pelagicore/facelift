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
#include "FaceliftIPCCommon.h"

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

    inline void sendPropertiesChanged(const QVariantMap &changedProperties);

    template<typename ... Args>
    void sendSignal(const QString& signalName, Args && ... args);

    template<typename ReturnType>
    void sendAsyncCallAnswer(LocalIPCMessage &replyMessage, const ReturnType returnValue);

    void sendAsyncCallAnswer(LocalIPCMessage &replyMessage);

    virtual IPCHandlingResult handleMethodCallMessage(LocalIPCMessage &requestMessage, LocalIPCMessage &replyMessage) = 0;

    virtual QVariantMap marshalProperties() = 0;

    virtual QVariant marshalProperty(const QString& propertyName) = 0;

    virtual void setProperty(const QString& propertyName, const QVariant& value) = 0;

    void registerService() override;

    void unregisterService() override;

    Q_SIGNAL void messageSent(LocalIPCMessage &message);

    void send(LocalIPCMessage &message);

    void sendReply(LocalIPCMessage &message);

    virtual void appendDBUSIntrospectionData(QTextStream &s) const = 0;

    QString introspect(const QString &path) const;

    template<typename T>
    MemberIDType memberID(T member, MemberIDType memberName) const
    {
        // Local member IDs are strings
        Q_UNUSED(member);
        return memberName;
    }

    template<typename T>
    T castFromQVariant(const QVariant& value) {
        return qvariant_cast<T>(value);
    }

    template<typename T, typename std::enable_if_t<!std::is_convertible<T, facelift::InterfaceBase*>::value, int> = 0>
    QVariant castToQVariant(const T& value) {
        return QVariant::fromValue(value);
    }

    template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::InterfaceBase*>::value, int> = 0>
    QVariant castToQVariant(const T& value) {
        QString  objectPath;
        if (value != nullptr) {
            objectPath = getOrCreateAdapter<typename std::remove_pointer<T>::type::IPCLocalAdapterType>(value)->objectPath();
        }
        return QVariant::fromValue(objectPath);
    }

    template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::InterfaceBase*>::value, int> = 0>
    QVariant castToQVariant(const QList<T>& value) {
        QStringList objectPathes;
        for (T service: value) {
            objectPathes.append(getOrCreateAdapter<typename std::remove_pointer<T>::type::IPCLocalAdapterType>(service)->objectPath());
        }
        return QVariant::fromValue(objectPathes);
    }

    template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::InterfaceBase*>::value, int> = 0>
    QVariant castToQVariant(const QMap<QString, T>& value) {
        QMap<QString, QString> objectPathesMap;
        for (const QString& key: value.keys()) {
            objectPathesMap[key] = getOrCreateAdapter<typename std::remove_pointer<T>::type::IPCLocalAdapterType>(value[key])->objectPath();
        }
        return QVariant::fromValue(objectPathesMap);
    }

protected:

    QString m_introspectionData;
    QString m_serviceName;

    bool m_previousReadyState = false;

    bool m_alreadyInitialized = false;
};

inline void LocalIPCServiceAdapterBase::sendPropertiesChanged(const QVariantMap &changedProperties )
{
    LocalIPCMessage reply(FaceliftIPCCommon::PROPERTIES_INTERFACE_NAME, FaceliftIPCCommon::PROPERTIES_CHANGED_SIGNAL_NAME);
    reply << interfaceName();
    reply << QVariant::fromValue(changedProperties);
    this->send(reply);
}

}

}
