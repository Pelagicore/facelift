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

#include "ipc-dbus.h"

#include <QObject>
#include <QDebug>
#include <QTextStream>
#include <QTimer>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusVirtualObject>
#include <QDBusAbstractInterface>
#include <QDBusInterface>
#include <QDBusServiceWatcher>

#include "FaceliftModel.h"
#include "utils.h"
#include "FaceliftProperty.h"

#include "QMLFrontend.h"
#include "QMLModel.h"


namespace facelift {
namespace dbus {



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

DBusIPCAdapterFactoryManager &DBusIPCAdapterFactoryManager::instance()
{
    static DBusIPCAdapterFactoryManager factory;
    return factory;
}

DBusIPCServiceAdapterBase *DBusIPCAttachedPropertyFactory::qmlAttachedProperties(QObject *object)
{
    auto provider = getProvider(object);

    DBusIPCServiceAdapterBase *serviceAdapter = nullptr;

    if (provider != nullptr) {
        auto interfaceID = provider->interfaceID();
        auto factory = DBusIPCAdapterFactoryManager::instance().getFactory(interfaceID);

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



bool DBusIPCServiceAdapterBase::handleMessage(const QDBusMessage &dbusMsg, const QDBusConnection &connection)
{
    DBusIPCMessage requestMessage(dbusMsg);

    DBusIPCMessage replyMessage = requestMessage.createReply();

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


void DBusIPCServiceAdapterBase::init(InterfaceBase *service)
{
    m_service = service;
    if (!m_alreadyInitialized) {
        if (!interfaceName().isEmpty()) {

            if (dbusManager().isDBusConnected()) {

                DBusManager::instance().registerServiceName(m_serviceName);

                qDebug() << "Registering IPC object at " << objectPath();
                m_alreadyInitialized = dbusManager().connection().registerVirtualObject(objectPath(), &m_dbusVirtualObject);
                if (m_alreadyInitialized) {
                    connect(service, &InterfaceBase::readyChanged, this, &DBusIPCServiceAdapterBase::onPropertyValueChanged);
                    connectSignals();
                } else {
                    qFatal("Could no register service at object path '%s'", qPrintable(objectPath()));
                }
            }

            registerLocalService();
        }
    }
}


void DBusIPCProxyBinder::bindToIPC()
{
    if ((m_serviceName != nullptr) && !m_interfaceName.isEmpty() && manager().isDBusConnected()) {

        qDebug() << "Initializing IPC proxy. objectPath:" << objectPath();

        m_busWatcher.addWatchedService(m_serviceName);
        m_busWatcher.setConnection(connection());
        QObject::connect(&m_busWatcher, &QDBusServiceWatcher::serviceRegistered, this, &DBusIPCProxyBinder::onServiceAvailable);

        auto successPropertyChangeSignal =
                connection().connect(m_serviceName, objectPath(), m_interfaceName,
                        DBusIPCServiceAdapterBase::PROPERTIES_CHANGED_SIGNAL_NAME,
                        this, SLOT(onPropertiesChanged(
                            const QDBusMessage&)));
        Q_ASSERT(successPropertyChangeSignal);

        auto successSignalTriggeredSignal =
                connection().connect(m_serviceName, objectPath(), m_interfaceName,
                        DBusIPCServiceAdapterBase::SIGNAL_TRIGGERED_SIGNAL_NAME,
                        this, SLOT(onSignalTriggered(
                            const QDBusMessage&)));
        Q_ASSERT(successSignalTriggeredSignal);

        requestPropertyValues();
    }

}

QString DBusIPCMessage::toString() const
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
}
