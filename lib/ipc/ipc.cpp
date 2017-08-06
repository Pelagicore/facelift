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
        qWarning() << "Can't register new object at path:" << id << "Previously registered object:" << m_registry[id]->service();
        Q_ASSERT(false);
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

DBusManager::DBusManager() : m_busConnection(QDBusConnection::sessionBus())
{
    m_dbusConnected = m_busConnection.isConnected();
    qDebug() << (m_dbusConnected ? "" : "NOT") << "connected to DBUS";
}

}
