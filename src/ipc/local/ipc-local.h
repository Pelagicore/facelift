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

#include "ipc-common/ipc-common.h"

namespace facelift {

class FaceliftIPCLibLocal_EXPORT LocalIPCMessage
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
    LocalIPCProxyBinder(InterfaceBase &owner, QObject *parent = nullptr) : IPCProxyBinderBase(owner, parent)
    {
    }

    void bindToIPC() override
    {
        // no actual IPC => Nothing to do
    }

};


template<typename InterfaceType, typename InterfaceAdapterType>
class InterfacePropertyIPCAdapterHandler
{
public:
    void update(IPCServiceAdapterBase *parent, InterfaceType *service)
    {
        Q_UNUSED(parent);
        Q_UNUSED(service);
    }

    QString generateObjectPath(const QString &parentPath)
    {
        Q_UNUSED(parentPath);
        return QString();
    }

    QString objectPath() const
    {
        return QString();
    }
};


template<typename ProxyType>
class InterfacePropertyIPCProxyHandler
{
public:
    void update(const QString &objectPath)
    {
        Q_UNUSED(objectPath);
    }

    ProxyType *getValue() const
    {
        return nullptr;
    }

    QString generateObjectPath(const QString &parentPath)
    {
        Q_UNUSED(parentPath);
        return QString();
    }
};


template<typename AdapterType, typename IPCAdapterType>
class LocalIPCProxy : public IPCProxyBase<AdapterType, IPCAdapterType>
{
    using IPCProxyBase<AdapterType, IPCAdapterType>::assignDefaultValue;

public:
    typedef qint32 MemberIDType;

    LocalIPCProxy(QObject *parent = nullptr) : IPCProxyBase<AdapterType, IPCAdapterType>(parent)
        , m_ipcBinder(*this)
    {
        this->initBinder(m_ipcBinder);
    }

    template<typename T>
    MemberIDType memberID(T id, const char *) const
    {
        return static_cast<MemberIDType>(id);
    }

    LocalIPCProxyBinder *ipc()
    {
        return &m_ipcBinder;
    }

    void connectToServer()
    {
        m_ipcBinder.connectToServer();
    }

    virtual void setServiceRegistered(bool isRegistered)
    {
        Q_UNUSED(isRegistered);
    }

    virtual void deserializeSignal(LocalIPCMessage &msg) = 0;

    virtual void deserializePropertyValues(LocalIPCMessage &msg)
    {
        Q_UNUSED(msg);
    };

    template<typename ... Args>
    void sendMethodCall(const char *methodName, const Args & ... /*args*/) const
    {
        qCritical() << "IPC unavailable for method" << methodName;
    }

    template<typename Type>
    void serializeValue(LocalIPCMessage &msg, const Type &v)
    {
        Q_UNUSED(msg);
        Q_UNUSED(v);
    }

    template<typename Type>
    void deserializeValue(LocalIPCMessage &msg, Type &v)
    {
        Q_UNUSED(msg);
        Q_UNUSED(v);
    }

    template<typename ReturnType, typename ... Args>
    void sendMethodCallWithReturn(MemberIDType memberID, ReturnType &returnValue, const Args & ... /*args*/) const
    {
        assignDefaultValue(returnValue);
        qCritical() << "IPC unavailable for method" << memberID;
    }

    template<typename PropertyType>
    void sendSetterCall(const char *methodName, const PropertyType &value) const
    {
        Q_UNUSED(value);
        qCritical() << "IPC unavailable for method" << methodName;
    }

    template<typename ElementType>
    void handleModelSignal(facelift::Model<ElementType> &model,
            facelift::MostRecentlyUsedCache<int, ElementType> &cache, const QString &modelName,
            const QString &signalName, LocalIPCMessage &msg)
    {
        Q_UNUSED(model);
        Q_UNUSED(cache);
        Q_UNUSED(modelName);
        Q_UNUSED(signalName);
        Q_UNUSED(msg);
    }

    template<typename ElementType>
    ElementType modelData(facelift::Model<ElementType> &model,
            facelift::MostRecentlyUsedCache<int, ElementType> &cache, const QString &modelName, int row)
    {
        Q_UNUSED(model);
        Q_UNUSED(cache);
        Q_UNUSED(modelName);
        Q_UNUSED(row);
        return ElementType();
    }

private:
    LocalIPCProxyBinder m_ipcBinder;
};


class FaceliftIPCLibLocal_EXPORT LocalIPCServiceAdapterBase : public IPCServiceAdapterBase
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
};


template<typename InterfaceType>
class LocalIPCServiceAdapter : public facelift::LocalIPCServiceAdapterBase
{
public:
    typedef InterfaceType TheServiceType;
    typedef qint32 MemberIDType;

    LocalIPCServiceAdapter(QObject *parent = nullptr) : facelift::LocalIPCServiceAdapterBase(parent)
    {
    }

    template<typename T>
    MemberIDType memberID(T id, const char *) const
    {
        return static_cast<MemberIDType>(id);
    }

    InterfaceType *service()
    {
        return m_service;
    }

    template<typename Type>
    void serializeValue(LocalIPCMessage &msg, const Type &v)
    {
        Q_UNUSED(msg);
        Q_UNUSED(v);
    }

    template<typename Type>
    void deserializeValue(LocalIPCMessage &msg, Type &v)
    {
        Q_UNUSED(msg);
        Q_UNUSED(v);
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

    virtual void serializePropertyValues(LocalIPCMessage &msg)
    {
        Q_UNUSED(msg);
    }

    template<typename ... Args>
    void sendSignal(MemberIDType memberID, const Args & ... /*args*/)
    {
        Q_UNUSED(memberID);
    }

    void connectModel(const QString &name, facelift::ModelBase &model)
    {
        Q_UNUSED(name);
        Q_UNUSED(model);
    }

    template<typename ElementType>
    void handleModelRequest(facelift::Model<ElementType> &model,
            LocalIPCMessage &requestMessage, LocalIPCMessage &replyMessage)
    {
        Q_UNUSED(model);
        Q_UNUSED(requestMessage);
        Q_UNUSED(replyMessage);
    }

    InterfaceBase *service() const override
    {
        return m_service;
    }

    void setService(QObject *service) override
    {
        m_service = bindToProvider<InterfaceType>(service);
    }

private:
    QPointer<InterfaceType> m_service;

};


class FaceliftIPCLibLocal_EXPORT LocalIPCAdapterFactoryManager
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


class FaceliftIPCLibLocal_EXPORT LocalIPCAttachedPropertyFactory : public IPCAttachedPropertyFactoryBase
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
                serviceAdapter->setEnabled(false);  // We disable by default to force people to write "IPC.enabled: true"
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
