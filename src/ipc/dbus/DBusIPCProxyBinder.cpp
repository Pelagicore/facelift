/**********************************************************************
**
** Copyright (C) 2019 Luxoft Sweden AB
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

#include <QDBusPendingCallWatcher>
#include "DBusIPCProxyBinder.h"
#include "DBusManagerInterface.h"
#include "DBusRequestHandler.h"
#include "DBusObjectRegistry.h"

namespace facelift {
namespace dbus {

DBusIPCProxyBinder::DBusIPCProxyBinder(DBusManagerInterface& dbusManager, InterfaceBase &owner, QObject *parent) :
    IPCProxyBinderBase(owner, parent),
    m_dbusManager(dbusManager),
    m_connection(QDBusConnection::sessionBus())
{
    m_busWatcher.setWatchMode(QDBusServiceWatcher::WatchForRegistration);
}

void DBusIPCProxyBinder::setServiceAvailable(bool isRegistered)
{
    if (m_serviceAvailable != isRegistered) {
        qCDebug(LogIpc) << "DBus service available:" << isRegistered << "objectPath:" << objectPath();
        m_serviceAvailable = isRegistered;
        emit serviceAvailableChanged();
    }
}

void DBusIPCProxyBinder::setInterfaceName(const QString &name)
{
    m_interfaceName = name;
    checkInit();
}

void DBusIPCProxyBinder::onServerNotAvailableError(const QString &propertyName) const
{
    qCCritical(LogIpc,
        "Error message received when calling method '%s' on service at path '%s'. "
        "This likely indicates that the server you are trying to access is not available yet",
        qPrintable(propertyName), qPrintable(objectPath()));
}

void DBusIPCProxyBinder::onPropertiesChanged(const QDBusMessage &dbusMessage)
{
    DBusIPCMessage msg(dbusMessage);
    QListIterator<QVariant> argumentsIterator(msg.arguments());
    QString interfaceName = (argumentsIterator.hasNext() ? castFromQVariant<QString>(argumentsIterator.next()): QString());
    if (interfaceName == m_interfaceName) {
        QVariantMap changedProperties = (argumentsIterator.hasNext() ? castFromQVariant<QVariantMap>(argumentsIterator.next()): QVariantMap());
        m_serviceObject->unmarshalPropertiesChanged(changedProperties);
    }
}

void DBusIPCProxyBinder::setHandler(DBusRequestHandler *handler)
{
    m_serviceObject = handler;
    checkInit();
}


void DBusIPCProxyBinder::requestPropertyValues()
{
    DBusIPCMessage msg(serviceName(), objectPath(), DBusIPCCommon::PROPERTIES_INTERFACE_NAME, DBusIPCCommon::GET_ALL_PROPERTIES_MESSAGE_NAME);
    msg << QVariant::fromValue(interfaceName());
    auto replyHandler = [this](DBusIPCMessage &replyMessage) {
        if (replyMessage.isReplyMessage()) {
            QVariantMap values = (!replyMessage.arguments().isEmpty() ? castFromQVariant<QVariantMap>(replyMessage.arguments().first()): QVariantMap());
            m_serviceObject->unmarshalProperties(values);
            m_serviceObject->setServiceRegistered(true);
        } else {
            qCDebug(LogIpc) << "Service not yet available : " << objectPath();
        }
    };

    if (isSynchronous()) {
        auto replyMessage = call(msg);
        replyHandler(replyMessage);
    } else {
        asyncCall(msg, this, replyHandler);
    }
}


void DBusIPCProxyBinder::onServiceNameKnown()
{
    auto successPropertyChangeSignal = m_connection.connect(m_serviceName,
                    objectPath(), DBusIPCCommon::PROPERTIES_INTERFACE_NAME, DBusIPCCommon::PROPERTIES_CHANGED_SIGNAL_NAME, this,
                    SLOT(onPropertiesChanged(const QDBusMessage&)));

    Q_UNUSED(successPropertyChangeSignal); // TODO: check

    for (const QString& signalEntry: m_serviceObject->getSignals()) {
        auto signalConnected = m_connection.connect(m_serviceName,
                                                  objectPath(), m_interfaceName, signalEntry, this, SLOT(handleGenericSignals(const QDBusMessage&)));
        Q_UNUSED(signalConnected) // TODO: check
    }

    m_busWatcher.addWatchedService(m_serviceName);
    m_busWatcher.setConnection(m_connection);
    QObject::connect(&m_busWatcher, &QDBusServiceWatcher::serviceRegistered, this, [this](){
        requestPropertyValues();
    });

    requestPropertyValues();
}

void DBusIPCProxyBinder::checkRegistry()
{
    if (isReadyToConnect()) {
        auto& objectRegistry = m_dbusManager.objectRegistry();
        auto registryObjects = objectRegistry.objects(isSynchronous());
        if (registryObjects.contains(objectPath())) {
            auto serviceName = registryObjects[objectPath()];
            if (serviceName != m_serviceName) {
                m_serviceName = serviceName;

                if (!m_serviceName.isEmpty() && !m_interfaceName.isEmpty() && m_connection.isConnected()) {
                    onServiceNameKnown();
                }
            }
        } else if (!m_serviceName.isEmpty()){ // no point to proceed on empty service name
            m_busWatcher.removeWatchedService(m_serviceName);

            m_connection.disconnect(m_serviceName,
                    objectPath(), DBusIPCCommon::PROPERTIES_INTERFACE_NAME, DBusIPCCommon::PROPERTIES_CHANGED_SIGNAL_NAME, this,
                    SLOT(onPropertiesChanged(const QDBusMessage&)));

            for (const QString& signalEntry: m_serviceObject->getSignals()) {
                m_connection.disconnect(m_serviceName,
                                      objectPath(), interfaceName(), signalEntry, this, SLOT(handleGenericSignals(const QDBusMessage&)));
            }

            setServiceAvailable(false);
            m_serviceName.clear();
        }
    }
}

void DBusIPCProxyBinder::handleGenericSignals(const QDBusMessage& msg)
{
    DBusIPCMessage dbusMsg(msg);
    m_serviceObject->handleSignals(dbusMsg);
}

void DBusIPCProxyBinder::asyncCall(DBusIPCMessage &message, const QObject *context, std::function<void(DBusIPCMessage &message)> callback)
{
    qCDebug(LogIpc) << "Sending async IPC message : " << message.toString();
    auto reply = new QDBusPendingCallWatcher(m_connection.asyncCall(message.outputMessage()));
    QObject::connect(reply, &QDBusPendingCallWatcher::finished, context, [callback, reply]() {
        DBusIPCMessage msg(reply->reply());
        if (msg.isReplyMessage()) {
            callback(msg);
        }
        reply->deleteLater();
    });
}

DBusIPCMessage DBusIPCProxyBinder::call(DBusIPCMessage &message) const
{
    qCDebug(LogIpc) << "Sending blocking IPC message : " << message.toString();
    auto replyDbusMessage = m_connection.call(message.outputMessage());
    DBusIPCMessage reply(replyDbusMessage);
    return reply;
}

void DBusIPCProxyBinder::setObjectPath(const QString &objectPath)
{
    if (!m_explicitServiceName) {
        m_dbusManager.objectRegistry().objects(false).addListener(objectPath, this, &DBusIPCProxyBinder::checkRegistry);
    }
    IPCProxyBinderBase::setObjectPath(objectPath);
}

void DBusIPCProxyBinder::setAddress(const QString& address)
{
    if (!address.isEmpty()) {
        m_connection = QDBusConnection::connectToBus(address, address);
        if (!m_connection.isConnected()) {
            qFatal("Can't connect to DBUS at address: %s", qPrintable(address));
        }
        IPCProxyBinderBase::setAddress(address);
    }
}
void DBusIPCProxyBinder::bindToIPC()
{
    if (m_explicitServiceName) {
        onServiceNameKnown();
    } else {
        checkRegistry();
    }

}


} // end namespace dbus
} // end namespace facelift
