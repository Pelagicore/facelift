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

namespace facelift {

void InterfaceManager::registerAdapter(const QString &objectPath, IPCServiceAdapterBase *adapter)
{
    Q_ASSERT(adapter);
    if (!m_registry.contains(objectPath)) {
        m_registry.insert(objectPath, adapter);
        QObject::connect(adapter, &IPCServiceAdapterBase::destroyed, this, &InterfaceManager::onAdapterDestroyed);
        emit adapterAvailable(adapter);
    } else {
        qFatal("Can't register new object at path: '%s'. Previously registered object: %s", qPrintable(objectPath),
                qPrintable(facelift::toString(*m_registry[objectPath]->service())));
    }
}

IPCServiceAdapterBase *InterfaceManager::getAdapter(const QString &objectPath)
{
    if (m_registry.contains(objectPath)) {
        return m_registry[objectPath];
    } else {
        return nullptr;
    }
}

void InterfaceManager::onAdapterDestroyed(IPCServiceAdapterBase *adapter)
{
    m_registry.remove(adapter->objectPath());
    emit adapterDestroyed(adapter);
}

InterfaceManager &InterfaceManager::instance()
{
    static InterfaceManager registry;
    return registry;
}

void IPCProxyBinderBase::onLocalAdapterAvailable(IPCServiceAdapterBase *adapter)
{
    if (adapter->objectPath() == this->objectPath()) {
        qDebug() << "Local server found for " << objectPath();
        m_inProcess = true;
        emit localAdapterAvailable(adapter);
    }
}

void IPCProxyBinderBase::connectToServer()
{
    if (!m_alreadyInitialized) {
        m_alreadyInitialized = true;
        QObject::connect(
            &InterfaceManager::instance(), &InterfaceManager::adapterAvailable, this,
            &IPCProxyBinderBase::onLocalAdapterAvailable);

        auto localAdapter = InterfaceManager::instance().getAdapter(this->objectPath());
        if (localAdapter != nullptr) {
            onLocalAdapterAvailable(localAdapter);
        } else {
            bindToIPC();
        }
    }
}

IPCAdapterFactoryManager &IPCAdapterFactoryManager::instance()
{
    static IPCAdapterFactoryManager factory;
    return factory;
}

IPCServiceAdapterBase *IPCAttachedPropertyFactory::qmlAttachedProperties(QObject *object)
{
    auto provider = getProvider(object);

    IPCServiceAdapterBase *serviceAdapter = nullptr;

    if (provider != nullptr) {
        auto interfaceID = provider->interfaceID();
        auto factory = IPCAdapterFactoryManager::instance().getFactory(interfaceID);

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

}
