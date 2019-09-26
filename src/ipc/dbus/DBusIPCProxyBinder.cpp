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
#include "DBusManager.h"
#include "DBusRequestHandler.h"
#include "ipc-dbus.h"
#include "ipc-dbus-object-registry.h"

namespace facelift {
namespace dbus {

DBusIPCProxyBinder::DBusIPCProxyBinder(InterfaceBase &owner, QObject *parent) :
    IPCProxyBinderBase(owner, parent),
    m_registry(DBusManager::instance().objectRegistry())
{
    m_busWatcher.setWatchMode(QDBusServiceWatcher::WatchForRegistration);
}

void DBusIPCProxyBinder::setServiceAvailable(bool isRegistered)
{
    if (m_serviceAvailable != isRegistered) {
        m_serviceAvailable = isRegistered;
        emit serviceAvailableChanged();
    }
}

void DBusIPCProxyBinder::setInterfaceName(const QString &name)
{
    m_interfaceName = name;
    checkInit();
}

void DBusIPCProxyBinder::onServerNotAvailableError(const char *methodName) const
{
    qCCritical(LogIpc,
        "Error message received when calling method '%s' on service at path '%s'. "
        "This likely indicates that the server you are trying to access is not available yet",
        qPrintable(methodName), qPrintable(objectPath()));
}

void DBusIPCProxyBinder::onPropertiesChanged(const QDBusMessage &dbusMessage)
{
    DBusIPCMessage msg(dbusMessage);
    m_serviceObject->deserializePropertyValues(msg, false);
}

void DBusIPCProxyBinder::onSignalTriggered(const QDBusMessage &dbusMessage)
{
    DBusIPCMessage msg(dbusMessage);
    m_serviceObject->deserializePropertyValues(msg, false);
    m_serviceObject->deserializeSignal(msg);
}

QDBusConnection &DBusIPCProxyBinder::connection() const
{
    return manager().connection();
}

DBusManager &DBusIPCProxyBinder::manager() const
{
    return DBusManager::instance();
}

void DBusIPCProxyBinder::setHandler(DBusRequestHandler *handler)
{
    m_serviceObject = handler;
    checkInit();
}


void DBusIPCProxyBinder::requestPropertyValues()
{
    DBusIPCMessage msg(serviceName(), objectPath(), interfaceName(), DBusIPCCommon::GET_PROPERTIES_MESSAGE_NAME);

    auto replyHandler = [this](DBusIPCMessage &replyMessage) {
        if (replyMessage.isReplyMessage()) {
            m_serviceObject->deserializePropertyValues(replyMessage, true);
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
    auto successPropertyChangeSignal = connection().connect(m_serviceName,
                    objectPath(), m_interfaceName, DBusIPCCommon::PROPERTIES_CHANGED_SIGNAL_NAME, this,
                    SLOT(onPropertiesChanged(const QDBusMessage&)));

    Q_UNUSED(successPropertyChangeSignal); // TODO: check

    auto successSignalTriggeredSignal = connection().connect(m_serviceName,
                    objectPath(), m_interfaceName, DBusIPCCommon::SIGNAL_TRIGGERED_SIGNAL_NAME, this,
                    SLOT(onSignalTriggered(const QDBusMessage&)));

    Q_UNUSED(successSignalTriggeredSignal); // TODO: check

    m_busWatcher.addWatchedService(m_serviceName);
    m_busWatcher.setConnection(connection());
    QObject::connect(&m_busWatcher, &QDBusServiceWatcher::serviceRegistered, this, [this](){
        requestPropertyValues();
    });

    requestPropertyValues();
}

void DBusIPCProxyBinder::checkRegistry()
{
    auto registryObjects = m_registry.objects(isSynchronous());
    if (registryObjects.contains(objectPath())) {
        auto serviceName = registryObjects[objectPath()];
        if (serviceName != m_serviceName) {
            m_serviceName = serviceName;

            if (!m_serviceName.isEmpty() && !m_interfaceName.isEmpty() && manager().isDBusConnected()) {
                onServiceNameKnown();
            }

        }
    } else {
        m_serviceName.clear();

        m_busWatcher.removeWatchedService(m_serviceName);

        connection().disconnect(m_serviceName,
                objectPath(), m_interfaceName, DBusIPCCommon::PROPERTIES_CHANGED_SIGNAL_NAME, this,
                SLOT(onPropertiesChanged(const QDBusMessage&)));

        connection().disconnect(m_serviceName,
                objectPath(), m_interfaceName, DBusIPCCommon::SIGNAL_TRIGGERED_SIGNAL_NAME, this,
                SLOT(onSignalTriggered(const QDBusMessage&)));

        setServiceAvailable(false);
    }
}

void DBusIPCProxyBinder::asyncCall(DBusIPCMessage &message, const QObject *context, std::function<void(DBusIPCMessage &message)> callback)
{
    qCDebug(LogIpc) << "Sending async IPC message : " << message.toString();
    auto reply = new QDBusPendingCallWatcher(connection().asyncCall(message.outputMessage()));
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
    auto replyDbusMessage = connection().call(message.outputMessage());
    DBusIPCMessage reply(replyDbusMessage);
    return reply;
}


void DBusIPCProxyBinder::bindToIPC()
{
    if (m_explicitServiceName) {
        onServiceNameKnown();
    } else {
        auto &registry = DBusManager::instance().objectRegistry();

        if (isSynchronous()) {
            registry.objects(true); // Read property value in a blocking way
        }
        QObject::connect(&registry, &DBusObjectRegistry::objectsChanged, this, [this] () {
            checkRegistry();
        }, Qt::UniqueConnection);

        checkRegistry();
    }

}

} // end namespace dbus
} // end namespace facelift
