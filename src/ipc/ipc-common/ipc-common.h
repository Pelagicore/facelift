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

#include "FaceliftLogging.h"
#include "FaceliftModel.h"
#include "QMLFrontend.h"
#include "QMLModel.h"

namespace facelift {

enum class CommonSignalID {
    readyChanged,
    firstSpecific
};

typedef int ASyncRequestID;

enum class IPCHandlingResult {
    OK,          // Message is successfully handled
    OK_ASYNC,    // Message is handled but it is an asynchronous request, so no reply should be sent for now
    INVALID,     // Message is invalid and could not be handled
};

class NewIPCServiceAdapterBase;


/**
 * This class maintains a registry of IPC services registered locally, which enables local proxies to get a direct reference to them
 */
class FaceliftIPCLibLocal_EXPORT InterfaceManager : public QObject
{
    Q_OBJECT

public:
    void registerAdapter(const QString &objectPath, NewIPCServiceAdapterBase *adapter);

    void unregisterAdapter(NewIPCServiceAdapterBase *adapter);

    NewIPCServiceAdapterBase *getAdapter(const QString &objectPath);

    Q_SIGNAL void adapterAvailable(NewIPCServiceAdapterBase *adapter);

    Q_SIGNAL void adapterUnavailable(QString objectPath, NewIPCServiceAdapterBase *adapter);

    static InterfaceManager &instance();

    static InterfaceBase * serviceMatches(const QString& objectPath, NewIPCServiceAdapterBase *adapter);

private:
    QMap<QString, QPointer<NewIPCServiceAdapterBase>> m_registry;

};

class FaceliftIPCLibLocal_EXPORT IPCServiceAdapterBase : public QObject
{
    Q_OBJECT

public:
    IPCServiceAdapterBase(QObject *parent = nullptr);

    const QString &interfaceName() const
    {
        return m_interfaceName;
    }

    void setInterfaceName(const QString &name)
    {
        m_interfaceName = name;
    }

    virtual void registerService() = 0;

    virtual void connectSignals() = 0;

    virtual InterfaceBase *service() const = 0;

    const QString &objectPath() const
    {
        return m_objectPath;
    }

    void setObjectPath(const QString &objectPath)
    {
        m_objectPath = objectPath;
    }

    QString generateObjectPath(const QString &parentPath) const;

    template<typename InterfaceType>
    typename InterfaceType::IPCDBusAdapterType *getOrCreateAdapter(InterfaceType *service)
    {
        using InterfaceAdapterType = typename InterfaceType::IPCDBusAdapterType ;

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
        serviceAdapter->registerService(this->generateObjectPath(this->objectPath()), service);
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
    QList<QPointer<IPCServiceAdapterBase> > m_subAdapters;
    QString m_objectPath;
    QString m_interfaceName;
};

class FaceliftIPCLibLocal_EXPORT IPCProxyBinderBase : public QObject
{
    Q_OBJECT

public:
    Q_PROPERTY(QString objectPath READ objectPath WRITE setObjectPath)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)

    IPCProxyBinderBase(InterfaceBase &owner, QObject *parent);

    bool enabled() const
    {
        return m_enabled;
    }

    void setEnabled(bool enabled)
    {
        m_enabled = enabled;
        checkInit();
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

    void onComponentCompleted();

    void checkInit();

    /**
     * Establish the connection with the server
     */
    void connectToServer();

    virtual void bindToIPC()
    {
    }

    Q_SIGNAL void complete();

    Q_SIGNAL void serviceAvailableChanged();

    InterfaceBase &owner()
    {
        return m_owner;
    }

    template<typename SubInterfaceType>
    typename SubInterfaceType::IPCDBusProxyType *getOrCreateSubProxy(const QString &objectPath)
    {
        using InterfaceProxyType = typename SubInterfaceType::IPCDBusProxyType;

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

    void setSynchronous(bool isSynchronous)
    {
        m_isSynchronous = isSynchronous;
    }

    bool isSynchronous() const
    {
        return m_isSynchronous;
    }

private:
    QMap<QString, IPCProxyBinderBase *> m_subProxies;
    QString m_objectPath;
    InterfaceBase &m_owner;
    bool m_alreadyInitialized = false;
    bool m_enabled = true;
    bool m_componentCompleted = false;
    bool m_isSynchronous = true;

};

template<typename Type>
inline void assignDefaultValue(Type &v)
{
    v = Type {};
}

template<typename AdapterType>
class IPCProxyBase : public AdapterType
{

public:
    using InterfaceType = AdapterType;

public:
    IPCProxyBase(QObject *parent) : AdapterType(parent)
    {
    }

    template<typename BinderType>
    void initBinder(BinderType &binder)
    {
        m_ipcBinder = &binder;
        QObject::connect(this, &InterfaceBase::componentCompleted, &binder, &BinderType::onComponentCompleted);
    }

    bool isSynchronous() const
    {
        return m_ipcBinder->isSynchronous();
    }

    bool ready() const override final
    {
        auto r = m_serviceReady;
        return r;
    }

    void setServiceReady(bool isServiceReady)
    {
        if (ready() != isServiceReady) {
            m_serviceReady = isServiceReady;
            emit this->readyChanged();
        }
    }

    virtual void emitChangeSignals()
    {
        emit this->readyChanged();
    }

    void deserializeCommonSignal(facelift::CommonSignalID signalID)
    {
        switch (signalID) {
        case facelift::CommonSignalID::readyChanged:
            emit this->readyChanged();
            break;
        default:
            qFatal("Unknown signal ID");
        }
    }

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


private:
    bool m_serviceReady = false;

protected:
    IPCProxyBinderBase *m_ipcBinder = nullptr;

};


class FaceliftIPCLibLocal_EXPORT IPCAttachedPropertyFactoryBase : public QObject
{
    Q_OBJECT

public:
    IPCAttachedPropertyFactoryBase(QObject *parent);

    static IPCServiceAdapterBase *qmlAttachedProperties(QObject *object);

    static InterfaceBase *getProvider(QObject *object);

};

enum class ModelUpdateEvent {
    DataChanged,
    Insert,
    Remove,
    Move,
    Reset
};


template<typename IPCAdapterType, typename ModelDataType>
class IPCAdapterModelPropertyHandler
{

public:
    IPCAdapterModelPropertyHandler(IPCAdapterType &adapter) : m_adapter(adapter)
    {
    }

    template<typename SignalID>
    void connectModel(SignalID signalID, facelift::Model<ModelDataType> &model)
    {
        m_model = &model;
        QObject::connect(m_model, static_cast<void (facelift::ModelBase::*)(int, int)>
            (&facelift::ModelBase::dataChanged), &m_adapter, [this, signalID] (int first, int last) {
            QList<ModelDataType> changedItems;
            for (int i = first ; i <= last ; i++) {
                changedItems.append(m_model->elementAt(i));
            }
            m_adapter.sendSignal(signalID, ModelUpdateEvent::DataChanged, first, changedItems);
        });
        QObject::connect(m_model, &facelift::ModelBase::beginRemoveElements, &m_adapter, [this] (int first, int last) {
            m_removeFirst = first;
            m_removeLast = last;
        });
        QObject::connect(m_model, &facelift::ModelBase::endRemoveElements, &m_adapter, [this, signalID] () {
            Q_ASSERT(m_removeFirst != UNDEFINED);
            Q_ASSERT(m_removeLast != UNDEFINED);
            m_adapter.sendSignal(signalID, ModelUpdateEvent::Remove, m_removeFirst, m_removeLast);
            m_removeFirst = UNDEFINED;
            m_removeLast = UNDEFINED;
        });
        QObject::connect(m_model, &facelift::ModelBase::beginMoveElements, &m_adapter, [this] (int sourceFirstIndex, int sourceLastIndex, int destinationIndex) {
            m_moveSourceFirstIndex = sourceFirstIndex;
            m_moveSourceLastIndex = sourceLastIndex;
            m_moveDestinationIndex = destinationIndex;
        });
        QObject::connect(m_model, &facelift::ModelBase::endMoveElements, &m_adapter, [this, signalID] () {
            Q_ASSERT(m_moveSourceFirstIndex != UNDEFINED);
            Q_ASSERT(m_moveSourceLastIndex != UNDEFINED);
            Q_ASSERT(m_moveDestinationIndex != UNDEFINED);
            m_adapter.sendSignal(signalID, ModelUpdateEvent::Move, m_moveSourceFirstIndex, m_moveSourceLastIndex, m_moveDestinationIndex);
            m_moveSourceFirstIndex = UNDEFINED;
            m_moveSourceLastIndex = UNDEFINED;
            m_moveDestinationIndex = UNDEFINED;
        });
        QObject::connect(m_model, &facelift::ModelBase::beginInsertElements, &m_adapter, [this] (int first, int last) {
            m_insertFirst = first;
            m_insertLast = last;
        });
        QObject::connect(m_model, &facelift::ModelBase::endInsertElements, &m_adapter, [this, signalID] () {
            Q_ASSERT(m_insertFirst != UNDEFINED);
            Q_ASSERT(m_insertLast != UNDEFINED);
            m_adapter.sendSignal(signalID, ModelUpdateEvent::Insert, m_insertFirst, m_insertLast);
            m_insertFirst = UNDEFINED;
            m_insertLast = UNDEFINED;
        });
        QObject::connect(m_model, &facelift::ModelBase::beginResetModel, &m_adapter, [this] () {
            m_resettingModel = true;
        });
        QObject::connect(m_model, &facelift::ModelBase::endResetModel, &m_adapter, [this, signalID] () {
            Q_ASSERT(m_resettingModel);
            m_adapter.sendSignal(signalID, ModelUpdateEvent::Reset, m_model->size());
            m_resettingModel = false;
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
    static constexpr int UNDEFINED = -1;
    IPCAdapterType &m_adapter;
    facelift::Model<ModelDataType> *m_model = nullptr;
    int m_removeFirst = UNDEFINED;
    int m_removeLast = UNDEFINED;
    int m_insertFirst = UNDEFINED;
    int m_insertLast = UNDEFINED;
    int m_moveSourceFirstIndex = UNDEFINED;
    int m_moveSourceLastIndex = UNDEFINED;
    int m_moveDestinationIndex = UNDEFINED;
    bool m_resettingModel = false;
};



template<typename IPCProxyType, typename ModelDataType>
class IPCProxyModelProperty : public facelift::ModelProperty<ModelDataType>
{

    using MemberID = typename IPCProxyType::MemberIDType;

public:
    IPCProxyModelProperty(IPCProxyType &proxy) : m_proxy(proxy), m_cache(PREFETCH_ITEM_COUNT * 10)
    {
    }

    template<typename IPCMessage>
    void handleSignal(IPCMessage &msg)
    {
        ModelUpdateEvent event;
        m_proxy.deserializeValue(msg, event);
        switch (event) {

        case ModelUpdateEvent::DataChanged:
        {
            int first;
            QList<ModelDataType> list;
            m_proxy.deserializeValue(msg, first);
            m_proxy.deserializeValue(msg, list);
            int last = first + list.size() - 1;
            for (int i = first; i <= last; ++i) {
                m_cache.insert(i, list.at(i - first));
            }
            emit this->dataChanged(first, last);
        } break;

        case ModelUpdateEvent::Insert:
        {
            int first, last;
            m_proxy.deserializeValue(msg, first);
            m_proxy.deserializeValue(msg, last);
            emit this->beginInsertElements(first, last);
            clear(); // TODO: insert elements in cache without clear()
            emit this->endInsertElements();
        } break;

        case ModelUpdateEvent::Remove:
        {
            int first, last;
            m_proxy.deserializeValue(msg, first);
            m_proxy.deserializeValue(msg, last);
            emit this->beginRemoveElements(first, last);
            m_cache.clear(); // TODO: remove elements from cache without clear()
            emit this->endRemoveElements();
        } break;

        case ModelUpdateEvent::Move:
        {
            int sourceFirstIndex, sourceLastIndex, destinationIndex;
            m_proxy.deserializeValue(msg, sourceFirstIndex);
            m_proxy.deserializeValue(msg, sourceLastIndex);
            m_proxy.deserializeValue(msg, destinationIndex);
            emit this->beginMoveElements(sourceFirstIndex, sourceLastIndex, destinationIndex);
            m_cache.clear(); // TODO: move elements in cache without clear()
            emit this->endMoveElements();
        } break;

        case ModelUpdateEvent::Reset:
        {
            emit this->beginResetModel();
            int size;
            m_proxy.deserializeValue(msg, size);
            this->setSize(size);
            clear();
            emit this->endResetModel();
        } break;

        }
    }

    void clear()
    {
        m_cache.clear();
        m_itemsRequestedFromServer.clear();
        m_itemsRequestedLocally.clear();
    }

    ModelDataType modelData(const MemberID &requestMemberID, int row)
    {
        ModelDataType retval {};
        if (m_cache.exists(row)) {
            retval = m_cache.get(row);
        } else {

            if (m_proxy.isSynchronous()) {

                int first = row > PREFETCH_ITEM_COUNT ? row - PREFETCH_ITEM_COUNT : 0;
                int last = row < this->size() - PREFETCH_ITEM_COUNT ? row + PREFETCH_ITEM_COUNT : this->size() - 1;

                while (m_cache.exists(first) && first < last) {
                    ++first;
                }
                while (m_cache.exists(last) && last > first) {
                    --last;
                }

                QList<ModelDataType> list;
                m_proxy.ipc()->sendMethodCallWithReturn(requestMemberID, list, first, last);

                Q_ASSERT(list.size() == (last - first + 1));

                for (int i = first; i <= last; ++i) {
                    m_cache.insert(i, list.at(i - first));
                }

                retval = list.at(row - first);
            } else {
                // Request items asynchronously and return a default item for now
                requestItemsAsync(requestMemberID, row);
                m_itemsRequestedLocally.append(row); // Remind that we delivered an invalid item for this index
            }
        }

        // Prefetch next items
        int nextIndex = std::min(this->size(), row + PREFETCH_ITEM_COUNT);
        if (!m_cache.exists(nextIndex) && !m_itemsRequestedFromServer.contains(nextIndex)) {
            requestItemsAsync(requestMemberID, nextIndex);
        }

        // Prefetch previous items
        int previousIndex = std::max(0, row - PREFETCH_ITEM_COUNT);
        if (!m_cache.exists(nextIndex) && !m_itemsRequestedFromServer.contains(previousIndex)) {
            requestItemsAsync(requestMemberID, previousIndex);
        }

        return retval;
    }

    /**
     * Request the items around the given index.
     */
    void requestItemsAsync(const MemberID &requestMemberID, int index)
    {
        // Find the first index which we should request, given what we already have in our cache
        int first = std::max(0, index - PREFETCH_ITEM_COUNT);
        while ((m_cache.exists(first) || m_itemsRequestedFromServer.contains(first)) && (first < this->size())) {
            ++first;
        }

        if ((first - index < PREFETCH_ITEM_COUNT) && (first != this->size())) {  // We don't request anything if the first index is outside the window
            int last = std::min(first + PREFETCH_ITEM_COUNT * 2, this->size() - 1);   // We query at least

            // Do not request the items from the end of the window, which we already have in our cache
            while ((m_cache.exists(last) || m_itemsRequestedFromServer.contains(last)) && (last >= first)) {
                --last;
            }

            if (first <= last) {
                m_proxy.ipc()->sendAsyncMethodCall(requestMemberID, facelift::AsyncAnswer<QList<ModelDataType> >(&m_proxy, [this, first,
                        last](QList<ModelDataType> list) {
                        //                    qCDebug(LogIpc) << "Received model items " << first << "-" << last;
                        for (int i = first; i <= last; ++i) {
                            auto &newItem = list[i - first];
                            if (!((m_cache.exists(i)) && (newItem == m_cache.get(i)))) {
                                m_cache.insert(i, newItem);
                                if (m_itemsRequestedLocally.contains(i)) {
                                    this->dataChanged(i);
                                    m_itemsRequestedLocally.removeAll(i);
                                }
                            }
                            m_itemsRequestedFromServer.removeAll(i);
                        }
                    }), first, last);
                for (int i = first; i <= last; ++i) {
                    m_itemsRequestedFromServer.append(i);
                }
            }
        }
    }

private:
    static constexpr int PREFETCH_ITEM_COUNT = 12;        // fetch 25 items around requested one

    IPCProxyType &m_proxy;
    facelift::MostRecentlyUsedCache<int, ModelDataType> m_cache;
    QList<int> m_itemsRequestedFromServer;
    QList<int> m_itemsRequestedLocally;
};


class FaceliftIPCLibLocal_EXPORT IPCAdapterFactoryManager
{
public:
    typedef NewIPCServiceAdapterBase * (*IPCAdapterFactory)(InterfaceBase *);

    static IPCAdapterFactoryManager &instance();

    template<typename AdapterType>
    static NewIPCServiceAdapterBase *createInstance(InterfaceBase *i)
    {
        auto adapter = new AdapterType(i);
        adapter->setService(i);
        qCDebug(LogIpc) << "Created adapter for interface " << i->interfaceID();
        return adapter;
    }

    template<typename AdapterType>
    static void registerType()
    {
        instance().registerType(AdapterType::TheServiceType::FULLY_QUALIFIED_INTERFACE_NAME,  &IPCAdapterFactoryManager::createInstance<AdapterType>);
    }

    void registerType(const QString &typeID, IPCAdapterFactory f);

    IPCAdapterFactory getFactory(const QString &typeID) const;

private:
    QMap<QString, IPCAdapterFactory> m_factories;
};

class FaceliftIPCLibLocal_EXPORT IPCAttachedPropertyFactory : public IPCAttachedPropertyFactoryBase
{
public:
    static NewIPCServiceAdapterBase *qmlAttachedProperties(QObject *object);

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
        v.clear();
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
        v.clear();
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
