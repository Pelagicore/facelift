/*
 *   This is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 *   Please see the LICENSE file for details.
 */

#include "ipc.h"
#include "QMLModel.h"

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
            provider = qmlImpl->interfac();
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



bool IPCServiceAdapterBase::handleMessage(const QDBusMessage &dbusMsg, const QDBusConnection &connection)
{
    IPCMessage requestMessage(dbusMsg);

    IPCMessage replyMessage = requestMessage.createReply();

    qDebug() << "Handling incoming message: " << requestMessage.toString();

    if (dbusMsg.interface() == INTROSPECTABLE_INTERFACE_NAME) {
        // TODO
    } else if (dbusMsg.interface() == PROPERTIES_INTERFACE_NAME) {
        // TODO
    } else {
        if (requestMessage.member() == GET_PROPERTIES_MESSAGE_NAME) {
            serializePropertyValues(replyMessage);
        } else {
            auto handlingResult = handleMethodCallMessage(requestMessage, replyMessage);
            if (handlingResult != IPCHandlingResult::OK) {
                replyMessage = requestMessage.createErrorReply("Invalid arguments", "TODO");
            }
        }
        replyMessage.send(connection);
        return true;
    }

    return false;
}


void IPCServiceAdapterBase::init(InterfaceBase *service)
{
    m_service = service;
    if (!m_alreadyInitialized && m_enabled && m_complete) {
        if ((service != nullptr) && !m_interfaceName.isEmpty() && !m_objectPath.isEmpty()) {

            if (dbusManager().isDBusConnected()) {

                DBusManager::instance().registerServiceName(m_serviceName);

                qDebug() << "Registering IPC object at " << m_objectPath;
                m_alreadyInitialized = dbusManager().connection().registerVirtualObject(m_objectPath, this);
                if (m_alreadyInitialized) {
                    connect(service, &InterfaceBase::readyChanged, this, &IPCServiceAdapterBase::onPropertyValueChanged);
                    connectSignals();
                } else {
                    qFatal("Could no register service at object path '%s'", qPrintable(m_objectPath));
                }
            }

            InterfaceManager::instance().registerAdapter(m_objectPath, *this);
        }
    }
}

void IPCProxyBinder::init()
{
    if (!m_alreadyInitialized && m_enabled && m_componentCompleted) {
        if ((m_serviceName != nullptr) && !m_interfaceName.isEmpty() && !m_objectPath.isEmpty()) {

            if (manager().isDBusConnected()) {

                qDebug() << "Initializing IPC proxy. objectPath:" << m_objectPath;

                m_busWatcher.addWatchedService(m_serviceName);
                m_busWatcher.setConnection(connection());
                connect(&m_busWatcher, &QDBusServiceWatcher::serviceRegistered, this, &IPCProxyBinder::onServiceAvailable);

                auto successPropertyChangeSignal =
                        connection().connect(m_serviceName, m_objectPath, m_interfaceName,
                                IPCServiceAdapterBase::PROPERTIES_CHANGED_SIGNAL_NAME,
                                this, SLOT(onPropertiesChanged(
                                    const QDBusMessage&)));
                Q_ASSERT(successPropertyChangeSignal);

                auto successSignalTriggeredSignal =
                        connection().connect(m_serviceName, m_objectPath, m_interfaceName,
                                IPCServiceAdapterBase::SIGNAL_TRIGGERED_SIGNAL_NAME,
                                this, SLOT(onSignalTriggered(
                                    const QDBusMessage&)));
                Q_ASSERT(successSignalTriggeredSignal);

                requestPropertyValues();
            }

            QObject::connect(
                &InterfaceManager::instance(), &InterfaceManager::adapterAvailable, this,
                &IPCProxyBinder::onLocalAdapterAvailable);

            m_alreadyInitialized = true;

            auto localAdapter = InterfaceManager::instance().getAdapter(this->objectPath());
            if (localAdapter != nullptr) {
                onLocalAdapterAvailable(localAdapter);
            }
        }
    }
}

void IPCProxyBinder::onLocalAdapterAvailable(IPCServiceAdapterBase *adapter)
{
    if (adapter->objectPath() == this->objectPath()) {
        qDebug() << "Local server found for " << objectPath();
        localAdapterAvailable(adapter);
    }
}

QString IPCMessage::toString() const
{
    QString str;
    QTextStream s(&str);

    s << "DBus message ";
    s << " service:" << m_message.service();
    s << " interface:" << m_message.interface();
    s << " path:" << m_message.path();
    s << " member:" << m_message.member();

    s << " / Arguments : [ ";
    for (auto &arg : m_message.arguments()) {
        s << arg.toString() << ", ";
    }
    s << " ]";

    s << " signature:" << m_message.signature();

    return str;
}

}
