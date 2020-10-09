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

#include "IPCDBusServiceAdapterBase.h"

namespace facelift {
namespace dbus {

bool IPCDBusServiceAdapterBase::handleMessage(const QDBusMessage &dbusMsg)
{
    DBusIPCMessage requestMessage(dbusMsg);
    DBusIPCMessage replyMessage = requestMessage.createReply();
    bool retVal = false;

    qCDebug(LogIpc) << "Handling incoming message: " << requestMessage.toString();

    if (dbusMsg.interface() == DBusIPCCommon::INTROSPECTABLE_INTERFACE_NAME) {
        // is handled via the QDBusVirtualObject
    } else if (dbusMsg.interface() == DBusIPCCommon::PROPERTIES_INTERFACE_NAME) {
        if (!m_signalsConnected) {
            m_signalsConnected = true;
            qCDebug(LogIpc) << "Enabling IPCDBusServiceAdapter for" << this->service();
            connectSignals();
        }
        if (dbusMsg.member() == DBusIPCCommon::GET_ALL_PROPERTIES_MESSAGE_NAME) {
            QListIterator<QVariant> argumentsIterator(dbusMsg.arguments());
            auto msgInterfaceName = (argumentsIterator.hasNext() ? castFromQVariant<QString>(argumentsIterator.next()): QString());
            if (msgInterfaceName == interfaceName()) {
                QVariantMap ret = marshalProperties();
                QMap<QString, QDBusVariant> convertedToDBusVariant;
                for (const QString& key: ret.keys()) {
                    convertedToDBusVariant[key] = QDBusVariant(ret[key]);
                }
                replyMessage << QVariant::fromValue(convertedToDBusVariant);
                send(replyMessage);
                retVal = true;
            }
        }
        else if (dbusMsg.member() == DBusIPCCommon::GET_PROPERTY_MESSAGE_NAME) {
            QListIterator<QVariant> argumentsIterator(dbusMsg.arguments());
            auto msgInterfaceName = (argumentsIterator.hasNext() ? castFromQVariant<QString>(argumentsIterator.next()): QString());
            if (msgInterfaceName == interfaceName()) {
                auto propertyName = (argumentsIterator.hasNext() ? castFromQVariant<QString>(argumentsIterator.next()): QString());
                QVariant value = marshalProperty(propertyName);
                if (value.isValid()) {
                    replyMessage << QVariant::fromValue(QDBusVariant(value));
                    send(replyMessage);
                    retVal = true;
                }
            }
        }
        else if (dbusMsg.member() == DBusIPCCommon::SET_PROPERTY_MESSAGE_NAME) {
            QListIterator<QVariant> argumentsIterator(requestMessage.arguments());
            auto msgInterfaceName = (argumentsIterator.hasNext() ? castFromQVariant<QString>(argumentsIterator.next()): QString());
            if (msgInterfaceName == interfaceName()) {
                QString propertyName = (argumentsIterator.hasNext() ? castFromQVariant<QString>(argumentsIterator.next()): QString());
                if (argumentsIterator.hasNext()) {
                    setProperty(propertyName, qdbus_cast<QDBusVariant>(argumentsIterator.next()).variant());
                    retVal = true;
                }
            }
            send(replyMessage);
        }
    } else if (dbusMsg.interface() == interfaceName()) {
        if (service()) {
            bool sendReply = true;
            auto handlingResult = handleMethodCallMessage(requestMessage, replyMessage);
            if (handlingResult == IPCHandlingResult::INVALID) {
                replyMessage = requestMessage.createErrorReply("Invalid arguments", "TODO");
            } else if (handlingResult == IPCHandlingResult::OK_ASYNC) {
                sendReply = false;
            }
            if (sendReply) {
                send(replyMessage);
            }
            retVal = true;
        } else {
            qCWarning(LogIpc) << "DBus request received for object which has been destroyed" << this;
        }
    }

    return retVal;
}

IPCDBusServiceAdapterBase::IPCDBusServiceAdapterBase(DBusManagerInterface& dbusManager, QObject *parent) :
    IPCServiceAdapterBase(parent),
    m_dbusVirtualObject(*this),
    m_dbusManager(dbusManager)
{
}

void IPCDBusServiceAdapterBase::sendAsyncCallAnswer(DBusIPCMessage &replyMessage)
{
    send(replyMessage);
}

void IPCDBusServiceAdapterBase::send(DBusIPCMessage &message)
{
    qCDebug(LogIpc) << "Sending IPC message : " << message.toString();
    bool successful = m_dbusManager.connection().send(message.outputMessage());
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

void IPCDBusServiceAdapterBase::unregisterService()
{
    if (m_alreadyInitialized) {
        m_dbusManager.connection().unregisterObject(objectPath());
        qCDebug(LogIpc) << "Unregistered IPCDBusServiceAdapter object at " << objectPath();
        m_dbusManager.objectRegistry().unregisterObject(objectPath());
        m_alreadyInitialized = false;
    }
}

void IPCDBusServiceAdapterBase::registerService()
{
    if (!m_alreadyInitialized) {
        if (m_dbusManager.isDBusConnected()) {

            if (!m_serviceName.isEmpty()) {
                m_dbusManager.registerServiceName(m_serviceName);
            }

            qCDebug(LogIpc) << "Registering IPCDBusServiceAdapter object at " << objectPath();
            m_alreadyInitialized = m_dbusManager.connection().registerVirtualObject(objectPath(), &m_dbusVirtualObject);
            if (!m_alreadyInitialized) {
                qFatal("Could not register service at object path '%s'", qPrintable(objectPath()));
            }

            m_dbusManager.objectRegistry().registerObject(objectPath(), facelift::AsyncAnswer<bool>(this, [](bool isSuccessful) {
                Q_ASSERT(isSuccessful);
            }));
        }
    }
}
}

}
