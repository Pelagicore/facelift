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
#include <QDBusPendingCallWatcher>

#include "FaceliftModel.h"
#include "FaceliftUtils.h"
#include "ModelProperty.h"

#include "QMLAdapter.h"
#include "QMLModel.h"

#include "DBusIPCProxy.h"
#include "DBusObjectRegistry.h"

#include "DBusManager.h"
#include "DBusIPCCommon.h"
#include "IPCDBusServiceAdapterBase.h"

namespace facelift {
namespace dbus {

constexpr const char *DBusIPCCommon::SIGNAL_TRIGGERED_SIGNAL_NAME;

void IPCDBusServiceAdapterBase::initOutgoingSignalMessage() {
    m_pendingOutgoingMessage = std::make_unique<DBusIPCMessage>(objectPath(), interfaceName(), DBusIPCCommon::SIGNAL_TRIGGERED_SIGNAL_NAME);

    // Send property value updates before the signal itself so that they are set before the signal is triggered on the client side.
    this->serializePropertyValues(*m_pendingOutgoingMessage, false);
}

void IPCDBusServiceAdapterBase::serializePropertyValues(DBusIPCMessage &msg, bool isCompleteSnapshot)
{
    Q_ASSERT(service());
    serializeOptionalValue(msg, service()->ready(), m_previousReadyState, isCompleteSnapshot);
}

void IPCDBusServiceAdapterBase::flush()
{
    if (m_pendingOutgoingMessage) {
        this->send(*m_pendingOutgoingMessage);
        m_pendingOutgoingMessage.reset();
    }
}

bool IPCDBusServiceAdapterBase::handleMessage(const QDBusMessage &dbusMsg)
{
    DBusIPCMessage requestMessage(dbusMsg);

    DBusIPCMessage replyMessage = requestMessage.createReply();

    qCDebug(LogIpc) << "Handling incoming message: " << requestMessage.toString();

    if (dbusMsg.interface() == DBusIPCCommon::INTROSPECTABLE_INTERFACE_NAME) {
        // TODO
    } else if (dbusMsg.interface() == DBusIPCCommon::PROPERTIES_INTERFACE_NAME) {
        // TODO
    } else {
        if (service()) {
            bool sendReply = true;
            if (requestMessage.member() == DBusIPCCommon::GET_PROPERTIES_MESSAGE_NAME) {
                if (!m_signalsConnected) {
                    m_signalsConnected = true;
                    QObject::connect(service(), &InterfaceBase::readyChanged, this, [this]() {
                        this->sendSignal(CommonSignalID::readyChanged);
                    });
                    qCDebug(LogIpc) << "Enabling IPCDBusServiceAdapter for" << this->service();
                    connectSignals();
                }
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
                send(replyMessage);
            }
            return true;
        } else {
            qCWarning(LogIpc) << "DBus request received for object which has been destroyed" << this;
        }
    }

    return false;
}

IPCDBusServiceAdapterBase::IPCDBusServiceAdapterBase(QObject *parent) : IPCServiceAdapterBase(parent), m_dbusVirtualObject(*this)
{
}

void IPCDBusServiceAdapterBase::sendAsyncCallAnswer(DBusIPCMessage &replyMessage)
{
    send(replyMessage);
}

void IPCDBusServiceAdapterBase::send(DBusIPCMessage &message)
{
    qCDebug(LogIpc) << "Sending IPC message : " << message.toString();
    bool successful = dbusManager().connection().send(message.outputMessage());
    Q_ASSERT(successful);
}


IPCDBusServiceAdapterBase::~IPCDBusServiceAdapterBase()
{
    emit destroyed(this);
    unregisterService();
}

QString IPCDBusServiceAdapterBase::introspect(const QString &path) const
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

    qCDebug(LogIpc) << "Introspection data for " << path << ":" << introspectionData;
    return introspectionData;
}

DBusManager &IPCDBusServiceAdapterBase::dbusManager()
{
    return DBusManager::instance();
}

void IPCDBusServiceAdapterBase::unregisterService()
{
    if (m_alreadyInitialized) {
        dbusManager().connection().unregisterObject(objectPath());
        qCDebug(LogIpc) << "Unregistered IPCDBusServiceAdapter object at " << objectPath();
        dbusManager().objectRegistry().unregisterObject(objectPath());
        m_alreadyInitialized = false;
    }
}

void IPCDBusServiceAdapterBase::registerService()
{
    if (!m_alreadyInitialized) {
        if (dbusManager().isDBusConnected()) {

            if (!m_serviceName.isEmpty()) {
                DBusManager::instance().registerServiceName(m_serviceName);
            }

            qCDebug(LogIpc) << "Registering IPCDBusServiceAdapter object at " << objectPath();
            m_alreadyInitialized = dbusManager().connection().registerVirtualObject(objectPath(), &m_dbusVirtualObject);
            if (!m_alreadyInitialized) {
                qFatal("Could not register service at object path '%s'", qPrintable(objectPath()));
            }

            DBusManager::instance().objectRegistry().registerObject(objectPath(), facelift::AsyncAnswer<bool>(this, [](bool isSuccessful) {
                Q_ASSERT(isSuccessful);
            }));
        }
    }
}
}

}
