/*
 *   This is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 *   Please see the LICENSE file for details.
 */

#include "ipc.h"

namespace facelift {

void InterfaceManager::registerAdapter(QString id, IPCServiceAdapterBase &adapter)
{
    if (!m_registry.contains(id)) {
        m_registry.insert(id, &adapter);
        QObject::connect(&adapter, &IPCServiceAdapterBase::destroyed, this, &InterfaceManager::onAdapterDestroyed);
        adapterAvailable(&adapter);
    } else {
        qFatal("Can't register new object at path: '%s'. Previously registered object: %s", qPrintable(id),
                qPrintable(facelift::toString(*m_registry[id]->service())));
    }
}

IPCServiceAdapterBase *InterfaceManager::getAdapter(QString id)
{
    if (m_registry.contains(id)) {
        return m_registry[id];
    } else {
        return nullptr;
    }
}

void InterfaceManager::onAdapterDestroyed(IPCServiceAdapterBase *adapter)
{
    m_registry.remove(adapter->objectPath());
    adapterDestroyed(adapter);
}

InterfaceManager &InterfaceManager::instance()
{
    static InterfaceManager registry;
    return registry;
}


DBusManager::DBusManager() : m_busConnection(QDBusConnection::sessionBus())
{
    m_dbusConnected = m_busConnection.isConnected();
    if (!m_dbusConnected) {
        qCritical() << "NOT connected to DBUS";
    }
}

DBusManager &DBusManager::instance()
{
    static DBusManager i;
    return i;
}

IPCAdapterFactoryManager &IPCAdapterFactoryManager::instance()
{
    static IPCAdapterFactoryManager factory;
    return factory;
}



IPCServiceAdapterBase *IPCAttachedPropertyFactory::qmlAttachedProperties(QObject *object)
{
    InterfaceBase *provider = nullptr;

    auto o = qobject_cast<facelift::QMLFrontendBase *>(object);
    if (o == nullptr) {
        auto qmlImpl = qobject_cast<facelift::ModelQMLImplementationBase *>(object);
        if (qmlImpl != nullptr) {
            provider = qmlImpl->interface();
        }
    } else {
        provider = o->provider();
    }

    IPCServiceAdapterBase *serviceAdapter = nullptr;

    if (provider != nullptr) {
        auto interfaceID = provider->interfaceID();
        auto factory = IPCAdapterFactoryManager::instance().getFactory(interfaceID);

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

}
