/*
 *   Copyright (C) 2017 Pelagicore AG
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 *   Please see the LICENSE file for details.
 */

#pragma once

#include "ipc-common/ipc-common.h"

namespace facelift {

class LocalIPCMessage
{

public:
    const QString &member() const
    {
        return m_s;
    }

private:
    QString m_s;
};

template<typename Type>
LocalIPCMessage &operator<<(LocalIPCMessage &msg, const Type &v)
{
    Q_UNUSED(v);
    return msg;
}

template<typename Type>
LocalIPCMessage &operator>>(LocalIPCMessage &msg, Type &v)
{
    Q_UNUSED(v);
    return msg;
}

template<typename Type>
LocalIPCMessage &operator>>(LocalIPCMessage &msg, facelift::Property<Type> &property)
{
    Q_UNUSED(property);
    return msg;
}


class LocalIPCProxyBinder : public IPCProxyBinderBase
{
    Q_OBJECT

public:
    LocalIPCProxyBinder(QObject *parent = nullptr) : IPCProxyBinderBase(parent)
    {
    }

    void bindToIPC() override
    {
        // no actual IPC => Nothing to do
    }

};


template<typename AdapterType, typename IPCAdapterType>
class LocalIPCProxy : public IPCProxyBase<AdapterType, IPCAdapterType>
{

public:
    LocalIPCProxy(QObject *parent = nullptr) : IPCProxyBase<AdapterType, IPCAdapterType>(parent)
    {
        this->initBinder(m_ipcBinder);
    }

    LocalIPCProxyBinder *ipc()
    {
        return &m_ipcBinder;
    }

    void connectToServer()
    {
        m_ipcBinder.connectToServer();
    }

    virtual void deserializeSignal(LocalIPCMessage &msg) = 0;
    virtual void deserializeSpecificPropertyValues(LocalIPCMessage &msg) = 0;

    template<typename ... Args>
    void sendMethodCall(const char *methodName, const Args & ... args)
    {
        Q_UNUSED(methodName);
        qCritical("IPC unavailable");
    }

    template<typename ReturnType, typename ... Args>
    void sendMethodCallWithReturn(const char *methodName, ReturnType &returnValue, const Args & ... args)
    {
        qCritical(
            "Error message received when calling method '%s' on service at path '%s'. This likely indicates that the server you are trying to access is not available yet",
            qPrintable(methodName), qPrintable(this->objectPath()));
    }

    template<typename PropertyType>
    void sendSetterCall(const char *methodName, const PropertyType &value)
    {
        qCritical(
            "Error message received when calling method '%s' on service at path '%s'. This likely indicates that the server you are trying to access is not available yet",
            qPrintable(methodName), qPrintable(this->objectPath()));
    }

private:
    LocalIPCProxyBinder m_ipcBinder;

};



class LocalIPCServiceAdapterBase : public IPCServiceAdapterBase
{

    Q_OBJECT

public:
    LocalIPCServiceAdapterBase(QObject *parent) : IPCServiceAdapterBase(parent)
    {
    }

    void init() override
    {
        registerLocalService();
    }

private:
};



template<typename InterfaceType>
class LocalIPCServiceAdapter : public facelift::LocalIPCServiceAdapterBase
{

public:
    typedef InterfaceType TheServiceType;

    LocalIPCServiceAdapter(QObject *parent = nullptr) : facelift::LocalIPCServiceAdapterBase(parent)
    {
    }

    InterfaceType *service()
    {
        return m_service;
    }

    virtual void appendDBUSIntrospectionData(QTextStream &s) const = 0;

    template<typename Type>
    void addPropertySignature(QTextStream &s, const char *propertyName, bool isReadonly) const
    {
        Q_UNUSED(s);
        Q_UNUSED(propertyName);
        Q_UNUSED(isReadonly);
    }

    template<typename ... Args>
    void addMethodSignature(QTextStream &s, const char *methodName, const std::array<const char *, sizeof ... (Args)> &argNames) const
    {
        Q_UNUSED(s);
        Q_UNUSED(methodName);
        Q_UNUSED(argNames);
    }

    template<typename ... Args>
    void addSignalSignature(QTextStream &s, const char *methodName, const std::array<const char *, sizeof ... (Args)> &argNames) const
    {
        Q_UNUSED(s);
        Q_UNUSED(methodName);
        Q_UNUSED(argNames);
    }

    virtual IPCHandlingResult handleMethodCallMessage(LocalIPCMessage &requestMessage, LocalIPCMessage &replyMessage) = 0;

    void onPropertyValueChanged()
    {
    }

    virtual void serializeSpecificPropertyValues(LocalIPCMessage &msg) = 0;

    template<typename ... Args>
    void sendSignal(const char *signalName, const Args & ... args)
    {
    }

    InterfaceBase *service() const override
    {
        return m_service;
    }

    void setService(InterfaceBase *service) override
    {
        m_service = toProvider<InterfaceType>(service);
    }

private:
    QPointer<InterfaceType> m_service;

};


class LocalIPCAdapterFactoryManager
{
public:
    typedef LocalIPCServiceAdapterBase * (*IPCAdapterFactory)(InterfaceBase *);

    static LocalIPCAdapterFactoryManager &instance();

    template<typename AdapterType>
    static LocalIPCServiceAdapterBase *createInstance(InterfaceBase *i)
    {
        auto adapter = new AdapterType(i);
        adapter->setService(i);
        qDebug() << "Created adapter for interface " << i->interfaceID();
        return adapter;
    }

    template<typename AdapterType>
    static void registerType()
    {
        auto &i = instance();
        const auto &typeID = AdapterType::TheServiceType::FULLY_QUALIFIED_INTERFACE_NAME;
        if (i.m_factories.contains(typeID)) {
            qWarning() << "IPC type already registered" << typeID;
        } else {
            i.m_factories.insert(typeID, &LocalIPCAdapterFactoryManager::createInstance<AdapterType>);
        }
    }

    IPCAdapterFactory getFactory(const QString &typeID) const
    {
        if (m_factories.contains(typeID)) {
            return m_factories[typeID];
        } else {
            return nullptr;
        }
    }

private:
    QMap<QString, IPCAdapterFactory> m_factories;

};

class LocalIPCAttachedPropertyFactory : public IPCAttachedPropertyFactoryBase
{
    Q_OBJECT

public:
    static IPCServiceAdapterBase *qmlAttachedProperties(QObject *object)
    {
        auto provider = getProvider(object);

        LocalIPCServiceAdapterBase *serviceAdapter = nullptr;

        if (provider != nullptr) {
            auto interfaceID = provider->interfaceID();
            auto factory = LocalIPCAdapterFactoryManager::instance().getFactory(interfaceID);

            if (factory != nullptr) {
                serviceAdapter = factory(provider);
            } else {
                qFatal("No factory found for interface '%s'", qPrintable(interfaceID));
            }
        } else {
            qFatal("Can't attach IPC to object with bad type: %s", object->metaObject()->className());
        }

        return serviceAdapter;
    }

};

}

QML_DECLARE_TYPEINFO(facelift::LocalIPCAttachedPropertyFactory, QML_HAS_ATTACHED_PROPERTIES)
