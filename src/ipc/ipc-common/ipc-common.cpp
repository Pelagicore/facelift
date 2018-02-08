#include "ipc-common.h"

namespace facelift {

void InterfaceManager::registerAdapter(const QString &objectPath, IPCServiceAdapterBase *adapter)
{
    Q_ASSERT(adapter);
    if (!m_registry.contains(objectPath)) {
        m_registry.insert(objectPath, adapter);
        QObject::connect(adapter, &IPCServiceAdapterBase::destroyed, this, &InterfaceManager::onAdapterDestroyed);
        adapterAvailable(adapter);
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
    adapterDestroyed(adapter);
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
        localAdapterAvailable(adapter);
    }
}
}
