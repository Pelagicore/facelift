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

#if defined(FaceliftIPCLibLocal_LIBRARY)
#  define FaceliftIPCLibLocal_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCLibLocal_EXPORT Q_DECL_IMPORT
#endif

#include <QDebug>

#include "FaceliftModel.h"
#include "QMLFrontend.h"
#include "QMLModel.h"

namespace facelift {

typedef int ASyncRequestID;

enum class IPCHandlingResult {
    OK,          // Message is successfully handled
    OK_ASYNC,    // Message is handled but it is an asynchronous request, so no reply should be sent for now
    INVALID,     // Message is invalid and could not be handled
};

class IPCServiceAdapterBase;


/**
 * This class maintains a registry of IPC services registered locally, which enables local proxies to get a direct reference to them
 */
class FaceliftIPCLibLocal_EXPORT InterfaceManager : public QObject
{
    Q_OBJECT

public:
    void registerAdapter(const QString &objectPath, IPCServiceAdapterBase *adapter);

    IPCServiceAdapterBase *getAdapter(const QString &objectPath);

    void onAdapterDestroyed(IPCServiceAdapterBase *object);

    Q_SIGNAL void adapterDestroyed(IPCServiceAdapterBase *adapter);
    Q_SIGNAL void adapterAvailable(IPCServiceAdapterBase *adapter);

    static InterfaceManager &instance();

private:
    QMap<QString, IPCServiceAdapterBase *> m_registry;

};

class FaceliftIPCLibLocal_EXPORT IPCServiceAdapterBase : public QObject
{

    Q_OBJECT

public:
    Q_PROPERTY(QObject * service READ service WRITE checkedSetService)
    Q_PROPERTY(QString objectPath READ objectPath WRITE setObjectPath)
    Q_PROPERTY(QString interfaceName READ interfaceName WRITE setInterfaceName)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)

    IPCServiceAdapterBase(QObject *parent = nullptr) : QObject(parent)
    {
    }

    bool enabled() const
    {
        return m_enabled;
    }

    void setEnabled(bool enabled)
    {
        m_enabled = enabled;
        onValueChanged();
    }

    virtual void init() = 0;

    virtual void connectSignals()
    {
    }

    virtual InterfaceBase *service() const = 0;

    void checkedSetService(QObject *service)
    {
        setService(service);
        onValueChanged();
    }

    virtual void setService(QObject *service) = 0;

    const QString &objectPath() const
    {
        return m_objectPath;
    }

    void setObjectPath(const QString &objectPath)
    {
        m_objectPath = objectPath;
        onValueChanged();
    }

    const QString &interfaceName() const
    {
        return m_interfaceName;
    }

    void setInterfaceName(const QString &name)
    {
        m_interfaceName = name;
        onValueChanged();
    }

    void onProviderCompleted()
    {
        // The parsing of the provider is finished => all our properties are set and we are ready to register our service
        m_providerReady = true;
        onValueChanged();
    }

    void onValueChanged()
    {
        if (enabled() && m_providerReady && !m_objectPath.isEmpty() && (service() != nullptr)) {
            init();
        }
    }

    void registerLocalService()
    {
        InterfaceManager::instance().registerAdapter(objectPath(), this);
    }

    QString generateObjectPath(const QString &parentPath) const
    {
        static int s_nextInstanceID = 0;
        QString path = parentPath + "/dynamic";
        path += QString::number(s_nextInstanceID++);
        return path;
    }

    Q_SIGNAL void destroyed(IPCServiceAdapterBase *adapter);

    template<typename ServiceType>
    ServiceType *bindToProvider(QObject *s)
    {
        auto service = qobject_cast<ServiceType *>(s);
        if (service == nullptr) {
            auto *qmlFrontend = qobject_cast<QMLFrontendBase *>(s);
            if (qmlFrontend != nullptr) {
                service = qobject_cast<ServiceType *>(qmlFrontend->providerPrivate());
            }
        }
        if (service != nullptr) {
            if (service->isComponentCompleted()) {
                onProviderCompleted();
            } else {
                QObject::connect(service, &InterfaceBase::componentCompleted, this, &IPCServiceAdapterBase::onProviderCompleted);
            }
        } else {
            qFatal("Bad service type : '%s'", qPrintable(facelift::toString(s)));
        }
        return service;
    }


    template<typename InterfaceType>
    typename InterfaceType::IPCAdapterType *getOrCreateAdapter(InterfaceType *service)
    {
        typedef typename InterfaceType::IPCAdapterType InterfaceAdapterType;

        if (service == nullptr) {
            return nullptr;
        }

        // Look for an existing adapter
        for (auto &adapter : m_subAdapters) {
            if (adapter->service() == service) {
                return qobject_cast<InterfaceAdapterType *>(adapter.data());
            }
        }

        auto serviceAdapter = new InterfaceAdapterType(service); // This object will be deleted together with the service itself
        serviceAdapter->setObjectPath(this->generateObjectPath(this->objectPath()));
        serviceAdapter->setService(service);
        serviceAdapter->init();
        m_subAdapters.append(serviceAdapter);

        return serviceAdapter;
    }

    template<typename InterfaceType, typename InterfaceAdapterType>
    class InterfacePropertyIPCAdapterHandler
    {

    public:
        void update(IPCServiceAdapterBase *parent, InterfaceType *service)
        {
            if (m_service != service) {
                m_service = service;
                m_serviceAdapter = parent->getOrCreateAdapter<InterfaceType>(service);
            }
        }

        QString objectPath() const
        {
            return (m_serviceAdapter ? m_serviceAdapter->objectPath() :  "");
        }

        QPointer<InterfaceType> m_service;
        QPointer<InterfaceAdapterType> m_serviceAdapter;
    };

private:
    QString m_objectPath;
    QString m_interfaceName;
    bool m_enabled = true;
    QList<QPointer<IPCServiceAdapterBase> > m_subAdapters;

protected:
    bool m_providerReady = false;

};


class FaceliftIPCLibLocal_EXPORT IPCProxyBinderBase : public QObject
{
    Q_OBJECT

public:
    Q_PROPERTY(QString objectPath READ objectPath WRITE setObjectPath)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)

    IPCProxyBinderBase(InterfaceBase &owner, QObject *parent) : QObject(parent), m_owner(owner)
    {
    }

    bool enabled() const
    {
        return m_enabled;
    }

    void setEnabled(bool enabled)
    {
        m_enabled = enabled;
        checkInit();
    }

    bool inProcess() const
    {
        return m_inProcess;
    }

    const QString &objectPath() const
    {
        return m_objectPath;
    }

    void setObjectPath(const QString &objectPath)
    {
        m_objectPath = objectPath;
        checkInit();
    }

    void onComponentCompleted()
    {
        m_componentCompleted = true;
        checkInit();
    }

    void checkInit()
    {
        if (m_componentCompleted && enabled() && !objectPath().isEmpty()) {
            this->connectToServer();
        }
    }

    /**
     * Establish the connection with the server
     */
    void connectToServer();

    virtual void bindToIPC() = 0;

    Q_SIGNAL void localAdapterAvailable(IPCServiceAdapterBase *adapter);

    void onLocalAdapterAvailable(IPCServiceAdapterBase *adapter);

    InterfaceBase &owner()
    {
        return m_owner;
    }

    template<typename InterfaceType>
    typename InterfaceType::IPCProxyType *getOrCreateSubProxy(const QString &objectPath)
    {
        typedef typename InterfaceType::IPCProxyType InterfaceProxyType;

        if (objectPath.isEmpty()) {
            return nullptr;
        }

        if (m_subProxies.contains(objectPath)) {
            return qobject_cast<InterfaceProxyType *>(&(m_subProxies[objectPath]->owner()));
        }

        auto proxy = new InterfaceProxyType();
        proxy->ipc()->setObjectPath(objectPath);
        proxy->connectToServer();

        m_subProxies.insert(objectPath, proxy->ipc());

        return proxy;
    }

    QMap<QString, IPCProxyBinderBase *> m_subProxies;

    void setSynchronous(bool isSynchronous)
    {
        m_isSynchronous = isSynchronous;
    }

    bool isSynchronous() const
    {
        return m_isSynchronous;
    }

protected:
    bool m_inProcess = false;

private:
    bool m_alreadyInitialized = false;
    QString m_objectPath;
    bool m_enabled = true;
    bool m_componentCompleted = false;
    InterfaceBase &m_owner;
    bool m_isSynchronous = true;

};


template<typename AdapterType, typename IPCAdapterType>
class IPCProxyBase : public AdapterType
{

public:
    typedef typename IPCAdapterType::TheServiceType InterfaceType;

public:
    IPCProxyBase(QObject *parent) : AdapterType(parent)
    {
        m_serviceReady.init(this, &InterfaceBase::readyChanged, "ready");
    }

    template<typename Type>
    void assignDefaultValue(Type &v) const
    {
        v = Type {};
    }

    template<typename BinderType>
    void initBinder(BinderType &binder)
    {
        m_ipcBinder = &binder;
        QObject::connect(&binder, &IPCProxyBinderBase::localAdapterAvailable, this, &IPCProxyBase::onLocalAdapterAvailable);
        QObject::connect(this, &InterfaceBase::componentCompleted, &binder, &BinderType::onComponentCompleted);
    }

    bool ready() const override final
    {
        if (localInterface() != nullptr) {
            return localInterface()->ready();
        }
        return m_serviceReady;
    }

    InterfaceType *localInterface() const
    {
        if (m_localAdapter) {
            return m_localAdapter->service();
        } else {
            return nullptr;
        }
    }

    void onLocalAdapterAvailable(IPCServiceAdapterBase *a)
    {
        auto adapter = qobject_cast<IPCAdapterType *>(a);
        if (adapter != nullptr) {
            m_localAdapter = adapter;
        }

        if (localInterface() != nullptr) {
            QObject::connect(localInterface(), &InterfaceBase::readyChanged, this, &AdapterType::readyChanged);
            bindLocalService(localInterface());
            m_serviceReady = localInterface()->ready();
        }
    }

    virtual void bindLocalService(InterfaceType *service) = 0;

    template<typename ProxyType>
    class InterfacePropertyIPCProxyHandler
    {

    public:
        InterfacePropertyIPCProxyHandler(IPCProxyBase &owner) : m_owner(owner)
        {
        }

        void update(const QString &objectPath)
        {
            if (m_proxy && (m_proxy->ipc()->objectPath() != objectPath)) {
                m_proxy = nullptr;
            }
            if (!m_proxy) {
                m_proxy = m_owner.m_ipcBinder->template getOrCreateSubProxy<ProxyType>(objectPath);
            }
        }

        ProxyType *getValue() const
        {
            return m_proxy;
        }

    private:
        QPointer<ProxyType> m_proxy;
        IPCProxyBase &m_owner;
    };


protected:
    void setReady(bool isReady)
    {
        m_serviceReady = isReady;
    }

    QPointer<IPCAdapterType> m_localAdapter;
    Property<bool> m_serviceReady;

    IPCProxyBinderBase *m_ipcBinder = nullptr;

};


class FaceliftIPCLibLocal_EXPORT IPCAttachedPropertyFactoryBase : public QObject
{
    Q_OBJECT

public:
    IPCAttachedPropertyFactoryBase(QObject *parent) : QObject(parent)
    {
    }

    static IPCServiceAdapterBase *qmlAttachedProperties(QObject *object);

    static InterfaceBase *getProvider(QObject *object)
    {
        InterfaceBase *provider = nullptr;

        auto o = qobject_cast<facelift::QMLFrontendBase *>(object);
        if (o == nullptr) {
            auto qmlImpl = qobject_cast<facelift::ModelQMLImplementationBase *>(object);
            if (qmlImpl != nullptr) {
                provider = qmlImpl->interfac();
            }
        } else {
            provider = o->providerPrivate();
        }

        return provider;
    }

};

enum class ModelUpdateEvent {
    DataChanged,
    BeginInsert,
    EndInsert,
    BeginRemove,
    EndRemove,
    BeginReset,
    EndReset
};


template<typename IPCAdapterType, typename ModelDataType>
class IPCAdapterModelPropertyHandler
{

public:
    IPCAdapterModelPropertyHandler(IPCAdapterType &adapter) : m_adapter(adapter)
    {
    }

    void connectModel(typename IPCAdapterType::MemberIDType memberID, facelift::Model<ModelDataType> &model)
    {
        m_model = &model;
        QObject::connect(m_model, static_cast<void (facelift::ModelBase::*)(int, int)>
            (&facelift::ModelBase::dataChanged), &m_adapter, [this, memberID] (int first, int last) {
            m_adapter.sendSignal(memberID, ModelUpdateEvent::DataChanged, first, last);
        });
        QObject::connect(m_model, &facelift::ModelBase::beginRemoveElements, &m_adapter, [this, memberID] (int first, int last) {
            m_adapter.sendSignal(memberID, ModelUpdateEvent::BeginRemove, first, last);
        });
        QObject::connect(m_model, &facelift::ModelBase::endRemoveElements, &m_adapter, [this, memberID] () {
            m_adapter.sendSignal(memberID, ModelUpdateEvent::EndRemove);
        });
        QObject::connect(m_model, &facelift::ModelBase::beginInsertElements, &m_adapter, [this, memberID] (int first, int last) {
            m_adapter.sendSignal(memberID, ModelUpdateEvent::BeginInsert, first, last);
        });
        QObject::connect(m_model, &facelift::ModelBase::endInsertElements, &m_adapter, [this, memberID] () {
            m_adapter.sendSignal(memberID, ModelUpdateEvent::EndInsert);
        });
        QObject::connect(m_model, &facelift::ModelBase::beginResetModel, &m_adapter, [this, memberID] () {
            m_adapter.sendSignal(memberID, ModelUpdateEvent::BeginReset);
        });
        QObject::connect(m_model, &facelift::ModelBase::endResetModel, &m_adapter, [this, memberID] () {
            m_adapter.sendSignal(memberID, ModelUpdateEvent::EndReset);
        });
    }

    template<typename IPCMessage>
    void handleModelRequest(IPCMessage &requestMessage, IPCMessage &replyMessage)
    {
        int first, last;
        m_adapter.deserializeValue(requestMessage, first);
        m_adapter.deserializeValue(requestMessage, last);
        QList<ModelDataType> list;

        // Make sure we do not request items which are out of range
        first = qMin(first, m_model->size() - 1);
        last = qMin(last, m_model->size() - 1);

        for (int i = first; i <= last; ++i) {
            list.append(m_model->elementAt(i));
        }

        m_adapter.serializeValue(replyMessage, list);
    }

private:
    IPCAdapterType &m_adapter;
    facelift::Model<ModelDataType> *m_model = nullptr;
};



template<typename IPCProxyType, typename ModelDataType>
class IPCProxyModelPropertyHandler
{

public:
    IPCProxyModelPropertyHandler(IPCProxyType &proxy, facelift::Model<ModelDataType> &model) : m_proxy(proxy), m_model(model)
    {
    }

    typedef typename IPCProxyType::MemberIDType MemberID;

    template<typename IPCMessage>
    void handleSignal(IPCMessage &msg)
    {
        ModelUpdateEvent event;
        m_proxy.deserializeValue(msg, event);
        switch (event) {

        case ModelUpdateEvent::DataChanged:
        {
            int first, last;
            m_proxy.deserializeValue(msg, first);
            m_proxy.deserializeValue(msg, last);
            for (int i = first; i <= last; ++i) {
                if (m_cache.exists(i)) {
                    m_cache.remove(i);
                }
            }
            emit m_model.dataChanged(first, last);
        } break;

        case ModelUpdateEvent::BeginInsert:
        {
            m_cache.clear();
            int first, last;
            m_proxy.deserializeValue(msg, first);
            m_proxy.deserializeValue(msg, last);
            emit m_model.beginInsertElements(first, last);
        } break;

        case ModelUpdateEvent::EndInsert:
        {
            emit m_model.endInsertElements();
        } break;

        case ModelUpdateEvent::BeginRemove:
        {
            m_cache.clear();
            int first, last;
            m_proxy.deserializeValue(msg, first);
            m_proxy.deserializeValue(msg, last);
            emit m_model.beginRemoveElements(first, last);
        } break;

        case ModelUpdateEvent::EndRemove:
        {
            emit m_model.endRemoveElements();
        } break;

        case ModelUpdateEvent::BeginReset:
        {
            emit m_model.beginResetModel();
            m_cache.clear();
        } break;

        case ModelUpdateEvent::EndReset:
        {
            m_cache.clear();
            emit m_model.endResetModel();
        } break;

        }
    }

    ModelDataType modelData(const typename IPCProxyType::MemberIDType &requestMemberID, int row)
    {
        ModelDataType retval;
        if (m_cache.exists(row)) {
            retval = m_cache.get(row);
        } else {
            static const int prefetch = 12;        // fetch 25 items around requested one
            QList<ModelDataType> list;
            int first = row > prefetch ? row - prefetch : 0;
            int last = row < m_model.size() - prefetch ? row + prefetch : m_model.size() - 1;

            while (m_cache.exists(first) && first < last) {
                ++first;
            }
            while (m_cache.exists(last) && last > first) {
                --last;
            }

            m_proxy.sendMethodCallWithReturn(requestMemberID, list, first, last);
            Q_ASSERT(list.size() == (last - first + 1));

            for (int i = first; i <= last; ++i) {
                m_cache.insert(i, list.at(i - first));
            }

            retval = list.at(row - first);
        }
        return retval;
    }

private:
    IPCProxyType &m_proxy;
    facelift::MostRecentlyUsedCache<int, ModelDataType> m_cache;
    facelift::Model<ModelDataType> &m_model;
};


class FaceliftIPCLibLocal_EXPORT IPCAdapterFactoryManager
{
public:
    typedef IPCServiceAdapterBase * (*IPCAdapterFactory)(InterfaceBase *);

    static IPCAdapterFactoryManager &instance();

    template<typename AdapterType>
    static IPCServiceAdapterBase *createInstance(InterfaceBase *i)
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
            i.m_factories.insert(typeID, &IPCAdapterFactoryManager::createInstance<AdapterType>);
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

class FaceliftIPCLibLocal_EXPORT IPCAttachedPropertyFactory : public IPCAttachedPropertyFactoryBase
{
public:
    static IPCServiceAdapterBase *qmlAttachedProperties(QObject *object);

};



template<typename Type, typename Enable = void>
struct IPCTypeRegisterHandler
{
    typedef Type SerializedType;

    template<typename OwnerType>
    static const Type &convertToSerializedType(const Type &v, OwnerType &adapter)
    {
        Q_UNUSED(adapter);
        return v;
    }

    template<typename OwnerType>
    static void convertToDeserializedType(Type &v, const SerializedType &serializedValue, OwnerType &adapter)
    {
        Q_UNUSED(adapter);
        v = serializedValue;
    }

};


template<typename Type>
struct IPCTypeRegisterHandler<QList<Type> >
{
    typedef QList<typename IPCTypeRegisterHandler<Type>::SerializedType> SerializedType;

    template<typename OwnerType>
    static SerializedType convertToSerializedType(const QList<Type> &v, OwnerType &adapter)
    {
        Q_UNUSED(v);
        Q_UNUSED(adapter);
        SerializedType convertedValue;
        for (const auto &e : v) {
            convertedValue.append(IPCTypeRegisterHandler<Type>::convertToSerializedType(e, adapter));
        }
        return convertedValue;
    }

    template<typename OwnerType>
    static void convertToDeserializedType(QList<Type> &v, const SerializedType &serializedValue, OwnerType &adapter)
    {
        for (const auto &e : serializedValue) {
            Type c;
            IPCTypeRegisterHandler<Type>::convertToDeserializedType(c, e, adapter);
            v.append(c);
        }
    }

};


template<typename Type>
struct IPCTypeRegisterHandler<QMap<QString, Type> >
{
    typedef QMap<QString, typename IPCTypeRegisterHandler<Type>::SerializedType> SerializedType;

    template<typename OwnerType>
    static SerializedType convertToSerializedType(const QMap<QString, Type> &v, OwnerType &adapter)
    {
        SerializedType convertedValue;
        for (const auto &key : v.keys()) {
            convertedValue.insert(key, IPCTypeRegisterHandler<Type>::convertToSerializedType(v[key], adapter));
        }
        return convertedValue;
    }

    template<typename OwnerType>
    static void convertToDeserializedType(QMap<QString, Type> &v, const SerializedType &serializedValue, OwnerType &adapter)
    {
        for (const auto &key : serializedValue.keys()) {
            Type c;
            IPCTypeRegisterHandler<Type>::convertToDeserializedType(c, serializedValue[key], adapter);
            v.insert(key, c);
        }
    }

};


template<typename Type>
struct IPCTypeRegisterHandler<Type *, typename std::enable_if<std::is_base_of<InterfaceBase, Type>::value>::type>
{
    typedef QString SerializedType;

    static SerializedType convertToSerializedType(Type *const &v, IPCServiceAdapterBase &adapter)
    {
        return adapter.getOrCreateAdapter<Type>(v)->objectPath();
    }

    template<typename OwnerType>
    static void convertToDeserializedType(Type * &v, const SerializedType &serializedValue, OwnerType &owner)
    {
        v = owner.template getOrCreateSubProxy<Type>(serializedValue);
    }

};


}

QML_DECLARE_TYPEINFO(facelift::IPCAttachedPropertyFactory, QML_HAS_ATTACHED_PROPERTIES)
