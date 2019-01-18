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
#include "FaceliftUtils.h"
#include "FaceliftProperty.h"

#include "QMLFrontend.h"
#include "QMLModel.h"

#include "DBusIPCProxy.h"
#include "ipc-dbus-object-registry.h"
#include "ipc-dbus-serialization.h"

namespace facelift {
namespace dbus {

struct FaceliftIPCLibDBus_EXPORT DBusIPCCommon {
    static constexpr const char *GET_PROPERTIES_MESSAGE_NAME = "GetAllProperties";
    static constexpr const char *PROPERTIES_CHANGED_SIGNAL_NAME = "PropertiesChanged";
    static constexpr const char *SIGNAL_TRIGGERED_SIGNAL_NAME = "SignalTriggered";
    static constexpr const char *SET_PROPERTY_MESSAGE_NAME = "SetProperty";
    static constexpr const char *INTROSPECTABLE_INTERFACE_NAME = "org.freedesktop.DBus.Introspectable";
    static constexpr const char *PROPERTIES_INTERFACE_NAME = "org.freedesktop.DBus.Properties";
};

constexpr const char *DBusIPCCommon::SIGNAL_TRIGGERED_SIGNAL_NAME;

void DBusIPCMessage::asyncCall(const QDBusConnection &connection, const QObject *context, std::function<void(DBusIPCMessage &message)> callback)
{
    if (m_outputPayload) {
        m_message << m_outputPayload->getContent();
    }
    qDebug() << "Sending async IPC message : " << toString();
    auto reply = new QDBusPendingCallWatcher(connection.asyncCall(m_message));
    QObject::connect(reply, &QDBusPendingCallWatcher::finished, context, [callback, reply]() {
        DBusIPCMessage msg(reply->reply());
        if (msg.isReplyMessage()) {
            callback(msg);
        }
        reply->deleteLater();
    });
}

DBusIPCMessage DBusIPCMessage::call(const QDBusConnection &connection)
{
    if (m_outputPayload) {
        m_message << m_outputPayload->getContent();
    }
    qDebug() << "Sending blocking IPC message : " << toString();
    auto replyDbusMessage = connection.call(m_message);
    DBusIPCMessage reply(replyDbusMessage);
    return reply;
}


void DBusIPCMessage::send(const QDBusConnection &connection)
{
    if (m_outputPayload) {
        m_message << m_outputPayload->getContent();
    }
    qDebug() << "Sending IPC message : " << toString();
    bool successful = connection.send(m_message);
    Q_ASSERT(successful);
}

DBusManager::DBusManager() : m_busConnection(QDBusConnection::sessionBus())
{
    m_dbusConnected = m_busConnection.isConnected();
    if (!m_dbusConnected) {
        qCritical() << "NOT connected to DBUS";
    }
}

DBusObjectRegistry &DBusManager::objectRegistry()
{
    if (m_objectRegistry == nullptr) {
        m_objectRegistry = new DBusObjectRegistry(*this);
        m_objectRegistry->init();
    }

    return *m_objectRegistry;
}

DBusManager &DBusManager::instance()
{
    static DBusManager i;
    return i;
}

void DBusIPCServiceAdapterBase::initOutgoingSignalMessage() {
    m_pendingOutgoingMessage = std::make_unique<DBusIPCMessage>(objectPath(), interfaceName(), DBusIPCCommon::SIGNAL_TRIGGERED_SIGNAL_NAME);

    // Send property value updates before the signal itself so that they are set before the signal is triggered on the client side.
    this->serializePropertyValues(*m_pendingOutgoingMessage, false);
}

void DBusIPCServiceAdapterBase::flush()
{
    if (m_pendingOutgoingMessage) {
        m_pendingOutgoingMessage->send(dbusManager().connection());
        m_pendingOutgoingMessage.reset();
    }
}

bool DBusIPCServiceAdapterBase::handleMessage(const QDBusMessage &dbusMsg, const QDBusConnection &connection)
{
    DBusIPCMessage requestMessage(dbusMsg);

    DBusIPCMessage replyMessage = requestMessage.createReply();

    qDebug() << "Handling incoming message: " << requestMessage.toString();

    if (dbusMsg.interface() == DBusIPCCommon::INTROSPECTABLE_INTERFACE_NAME) {
        // TODO
    } else if (dbusMsg.interface() == DBusIPCCommon::PROPERTIES_INTERFACE_NAME) {
        // TODO
    } else {
        bool sendReply = true;
        if (requestMessage.member() == DBusIPCCommon::GET_PROPERTIES_MESSAGE_NAME) {
            serializePropertyValues(replyMessage, true);
        } else {
            auto handlingResult = handleMethodCallMessage(requestMessage, replyMessage);
            if (handlingResult == IPCHandlingResult::INVALID) {
                replyMessage = requestMessage.createErrorReply("Invalid arguments", "TODO");
            } else if (handlingResult == IPCHandlingResult::OK_ASYNC) {
                sendReply = false;
            }
        }
        if (sendReply) {
            replyMessage.send(connection);
        }
        return true;
    }

    return false;
}

DBusIPCServiceAdapterBase::DBusIPCServiceAdapterBase(QObject *parent) : IPCServiceAdapterBase(parent), m_dbusVirtualObject(*this)
{
}

DBusIPCServiceAdapterBase::~DBusIPCServiceAdapterBase()
{
    emit destroyed(this);
    if (m_alreadyInitialized) {
        DBusManager::instance().objectRegistry().unregisterObject(objectPath());
    }
}

QString DBusIPCServiceAdapterBase::introspect(const QString &path) const
{
    QString introspectionData;

    if (path == objectPath()) {
        QTextStream s(&introspectionData);
        s << "<interface name=\"" << interfaceName() << "\">";
        appendDBUSIntrospectionData(s);
        s << "</interface>";
    } else {
        qFatal("Wrong object path");
    }

    qDebug() << "Introspection data for " << path << ":" << introspectionData;
    return introspectionData;
}

void DBusIPCServiceAdapterBase::init()
{
    if (!m_alreadyInitialized) {
        if (dbusManager().isDBusConnected()) {

            if (!m_serviceName.isEmpty()) {
                DBusManager::instance().registerServiceName(m_serviceName);
            }

            qDebug() << "Registering IPC object at " << objectPath();
            m_alreadyInitialized = dbusManager().connection().registerVirtualObject(objectPath(), &m_dbusVirtualObject);
            if (m_alreadyInitialized) {
                QObject::connect(service(), &InterfaceBase::readyChanged, this, [this]() {
                    this->sendSignal(CommonSignalID::readyChanged);
                });
                connectSignals();
            } else {
                qFatal("Could no register service at object path '%s'", qPrintable(objectPath()));
            }

            DBusManager::instance().objectRegistry().registerObject(objectPath(), facelift::AsyncAnswer<bool>(this, [](bool isSuccessful) {
                Q_ASSERT(isSuccessful);
            }));
        }
    }
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

void DBusIPCProxyBinder::requestPropertyValues()
{
    DBusIPCMessage msg(serviceName(), objectPath(), interfaceName(), DBusIPCCommon::GET_PROPERTIES_MESSAGE_NAME);

    auto replyHandler = [this](DBusIPCMessage &replyMessage) {
        if (replyMessage.isReplyMessage()) {
            m_serviceObject->deserializePropertyValues(replyMessage, true);
            m_serviceObject->setServiceRegistered(true);
        } else {
            qDebug() << "Service not yet available : " << objectPath();
        }
    };

    if (isSynchronous()) {
        auto replyMessage = msg.call(connection());
        replyHandler(replyMessage);
    } else {
        msg.asyncCall(connection(), this, replyHandler);
    }
}


void DBusIPCProxyBinder::onServiceAvailable()
{
    requestPropertyValues();
}

void DBusIPCProxyBinder::bindToIPC()
{
    if (!m_explicitServiceName) {
        auto &registry = DBusManager::instance().objectRegistry();

        auto registryObjects = registry.objects(isSynchronous());
        if (registryObjects.contains(objectPath())) {
            m_serviceName = registryObjects[objectPath()];
        }

        QObject::connect(&registry, &DBusObjectRegistry::objectsChanged, this, [this, &registry] () {
            auto registryObjects = registry.objects(isSynchronous());
            if (registryObjects.contains(objectPath())) {
                auto serviceName = registryObjects[objectPath()];
                if (serviceName != m_serviceName) {
                    m_serviceName = serviceName;
                    bindToIPC();
                }
            } else {
                m_serviceName.clear();
            }
        });

    }

    if (!m_serviceName.isEmpty() && !m_interfaceName.isEmpty() && manager().isDBusConnected()) {

        qDebug() << "Initializing IPC proxy. objectPath:" << objectPath() << "Service name:" << m_serviceName;

        m_busWatcher.addWatchedService(m_serviceName);
        m_busWatcher.setConnection(connection());
        QObject::connect(&m_busWatcher, &QDBusServiceWatcher::serviceRegistered, this, &DBusIPCProxyBinder::onServiceAvailable);

        auto successPropertyChangeSignal =
                connection().connect(m_serviceName,
                        objectPath(), m_interfaceName, DBusIPCCommon::PROPERTIES_CHANGED_SIGNAL_NAME, this,
                        SLOT(onPropertiesChanged(const QDBusMessage&)));
        Q_ASSERT(successPropertyChangeSignal);

        auto successSignalTriggeredSignal =
                connection().connect(m_serviceName,
                        objectPath(), m_interfaceName, DBusIPCCommon::SIGNAL_TRIGGERED_SIGNAL_NAME, this,
                        SLOT(onSignalTriggered(const QDBusMessage&)));
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

OutputPayLoad &DBusIPCMessage::outputPayLoad()
{
    if (m_outputPayload == nullptr) {
        m_outputPayload = std::make_unique<OutputPayLoad>();
    }
    return *m_outputPayload;
}

InputPayLoad &DBusIPCMessage::inputPayLoad()
{
    if (m_inputPayload == nullptr) {
        auto byteArray = m_message.arguments()[0].value<QByteArray>();
        m_inputPayload = std::make_unique<InputPayLoad>(byteArray);
    }
    return *m_inputPayload;
}


}

}
