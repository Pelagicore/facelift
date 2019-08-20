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

#include "ipc-common.h"
#include "IPCServiceAdapterBase.h"
#include "IPCProxyNewBase.h"
#include "FaceliftStringConversion.h"

namespace facelift {

Q_LOGGING_CATEGORY(LogIpc, "facelift.ipc")

IPCProxyBinderBase::IPCProxyBinderBase(InterfaceBase &owner, QObject *parent) : QObject(parent), m_owner(owner)
{
}

IPCServiceAdapterBase::IPCServiceAdapterBase(QObject *parent) : QObject(parent)
{
}

QString IPCServiceAdapterBase::generateObjectPath(const QString &parentPath) const
{
    static int s_nextInstanceID = 0;
    QString path = parentPath + "/dynamic";
    path += QString::number(s_nextInstanceID++);
    return path;
}

void InterfaceManager::registerAdapter(const QString &objectPath, NewIPCServiceAdapterBase *adapter)
{
    Q_ASSERT(adapter);
    if ((!m_registry.contains(objectPath)) || (m_registry[objectPath] == nullptr)) {
        m_registry.insert(objectPath, adapter);
        emit adapterAvailable(adapter);
    } else {
        qFatal("Can't register new object at path: '%s'. Previously registered object: %s", qPrintable(objectPath),
                qPrintable(facelift::toString(m_registry[objectPath]->service())));
    }
}

void InterfaceManager::unregisterAdapter(NewIPCServiceAdapterBase *adapter) {
    for (auto& key : m_registry.keys()) {
        if (m_registry[key] == adapter) {
            m_registry.remove(key);
            emit adapterUnavailable(key, adapter);
            qCDebug(LogIpc) << "IPC service unregistered" << adapter;
            break;
        }
    }
}

InterfaceBase * InterfaceManager::serviceMatches(const QString& objectPath, NewIPCServiceAdapterBase *adapter) {
    if (adapter->objectPath() == objectPath) {
        return adapter->service();
    } else {
        return nullptr;
    }
}

NewIPCServiceAdapterBase *InterfaceManager::getAdapter(const QString &objectPath)
{
    if (m_registry.contains(objectPath)) {
        return m_registry[objectPath];
    } else {
        return nullptr;
    }
}

InterfaceManager &InterfaceManager::instance()
{
    static auto manager = new InterfaceManager(); // TODO : fix memory leak
    return *manager;
}

void IPCProxyBinderBase::connectToServer()
{
    if (!m_alreadyInitialized) {
        m_alreadyInitialized = true;
        bindToIPC();
    }
}

void IPCProxyBinderBase::checkInit()
{
    if (m_componentCompleted && enabled() && !objectPath().isEmpty()) {
        this->connectToServer();
        emit complete();
    }
}

void IPCProxyBinderBase::onComponentCompleted()
{
    m_componentCompleted = true;
    checkInit();
}

IPCAdapterFactoryManager::IPCAdapterFactory IPCAdapterFactoryManager::getFactory(const QString &typeID) const
{
    if (m_factories.contains(typeID)) {
        return m_factories[typeID];
    } else {
        return nullptr;
    }
}

void IPCAdapterFactoryManager::registerType(const QString &typeID, IPCAdapterFactory f) {
    if (m_factories.contains(typeID)) {
        qCDebug(LogIpc) << "IPC type already registered" << typeID;
    } else {
        m_factories.insert(typeID, f);
    }
}

IPCAdapterFactoryManager &IPCAdapterFactoryManager::instance()
{
    static IPCAdapterFactoryManager factory;
    return factory;
}

NewIPCServiceAdapterBase *IPCAttachedPropertyFactory::qmlAttachedProperties(QObject *object)
{
    auto provider = getProvider(object);

    NewIPCServiceAdapterBase *serviceAdapter = nullptr;

    if (provider != nullptr) {
        auto interfaceID = provider->interfaceID();
        auto factory = IPCAdapterFactoryManager::instance().getFactory(interfaceID);

        if (factory != nullptr) {
            serviceAdapter = factory(provider);
            serviceAdapter->setEnabled(false);  // We disable by default to force people to write "IPC.enabled: true"
        } else {
            qFatal("No IPC factory found for interface '%s'. Missing '@ipc-sync' or '@ipc-async' decorators ?", qPrintable(interfaceID));
        }
    } else {
        qFatal("Can't attach IPC to object with bad type: %s", object->metaObject()->className());
    }

    return serviceAdapter;
}

IPCAttachedPropertyFactoryBase::IPCAttachedPropertyFactoryBase(QObject *parent) : QObject(parent)
{
}

InterfaceBase *IPCAttachedPropertyFactoryBase::getProvider(QObject *object)
{
    InterfaceBase *provider = nullptr;

    auto o = qobject_cast<facelift::QMLAdapterBase *>(object);
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

void NotAvailableImplBase::logMethodCall(const InterfaceBase &i, const char *methodName)
{
    qCCritical(LogIpc).nospace().noquote() << "Can not call method \"" << methodName << "(...)\" on IPC proxy for interface \"" << i.interfaceID()
                                           << "\". Make sure that the corresponding server is registered";
}

void NotAvailableImplBase::logSetterCall(const InterfaceBase &i, const char *propertyName)
{
    qCCritical(LogIpc).nospace().noquote() << "Can not call setter of property \"" << propertyName << "\" on IPC proxy for interface \"" << i.interfaceID()
                                           << "\". Make sure that the corresponding server is registered";
}

void NotAvailableImplBase::logGetterCall(const InterfaceBase &i, const char *propertyName)
{
    qCDebug(LogIpc) << "Getter of property" << propertyName << "is called" << i.interfaceID() << &i << i.interfaceID();
}

}
