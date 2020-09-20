/**********************************************************************
**
** Copyright (C) 2020 Luxoft Sweden AB
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

#include "LocalIPCMessage.h"

#include "LocalIPCServiceAdapterBase.h"
#include "LocalIPC-serialization.h"
#include "LocalIPCRegistry.h"


namespace facelift {
namespace local {

IPCHandlingResult LocalIPCServiceAdapterBase::handleMessage(LocalIPCMessage &requestMessage)
{
    LocalIPCMessage replyMessage = requestMessage.createReply();

    qCDebug(LogIpc) << "Handling incoming message: " << requestMessage.toString();

    auto handlingResult = IPCHandlingResult::OK;

    bool sendReply = true;
    if (requestMessage.interface() == FaceliftIPCCommon::PROPERTIES_INTERFACE_NAME) {
        if (requestMessage.member() == FaceliftIPCCommon::GET_ALL_PROPERTIES_MESSAGE_NAME) {
            replyMessage << QVariant::fromValue(marshalProperties());
        }
        else if (requestMessage.member() == FaceliftIPCCommon::GET_PROPERTY_MESSAGE_NAME) {
            QListIterator<QVariant> argumentsIterator(requestMessage.arguments());
            auto msgInterfaceName = (argumentsIterator.hasNext() ? castFromQVariant<QString>(argumentsIterator.next()): QString());
            // no need to check interface name in local variant
            auto propertyName = (argumentsIterator.hasNext() ? castFromQVariant<QString>(argumentsIterator.next()): QString());
            QVariant value = marshalProperty(propertyName);
            replyMessage << value;
            send(replyMessage);
        }
        else if (requestMessage.member() == FaceliftIPCCommon::SET_PROPERTY_MESSAGE_NAME) {
            QListIterator<QVariant> argumentsIterator(requestMessage.arguments());
            auto msgInterfaceName = (argumentsIterator.hasNext() ? castFromQVariant<QString>(argumentsIterator.next()): QString());
            if (msgInterfaceName == interfaceName()) {
                QString propertyName = (argumentsIterator.hasNext() ? castFromQVariant<QString>(argumentsIterator.next()): QString());
                if (argumentsIterator.hasNext()) {
                    setProperty(propertyName, argumentsIterator.next());
                }
            }
        }
    }
    else {
        handlingResult = handleMethodCallMessage(requestMessage, replyMessage);
        if (handlingResult == IPCHandlingResult::INVALID) {
            replyMessage = requestMessage.createErrorReply();
        } else if (handlingResult == IPCHandlingResult::OK_ASYNC) {
            sendReply = false;
        }
    }

    if (sendReply) {
        replyMessage.notifyListener();
    }

    return handlingResult;
}

LocalIPCServiceAdapterBase::LocalIPCServiceAdapterBase(QObject *parent) : IPCServiceAdapterBase(parent)
{
}

void LocalIPCServiceAdapterBase::sendAsyncCallAnswer(LocalIPCMessage &replyMessage)
{
    sendReply(replyMessage);
}


LocalIPCServiceAdapterBase::~LocalIPCServiceAdapterBase()
{
    emit destroyed(this); // TODO : get rid of this
    unregisterService();
}

QString LocalIPCServiceAdapterBase::introspect(const QString &path) const
{
    Q_UNUSED(path);
    return QString();
}

void LocalIPCServiceAdapterBase::unregisterService()
{
    if (m_alreadyInitialized) {
        LocalIPCRegistry::instance().unregisterAdapter(this);
        m_alreadyInitialized = false;
    }
}

void LocalIPCServiceAdapterBase::registerService()
{
    if (!m_alreadyInitialized) {
        LocalIPCRegistry::instance().registerAdapter(objectPath(), this);
        m_alreadyInitialized = true;
        qCDebug(LogIpc) << "Registering local IPC object at " << objectPath();
        if (m_alreadyInitialized) {
            connectSignals();
        } else {
            qFatal("Could not register service at object path '%s'", qPrintable(objectPath()));
        }
    }
}

void LocalIPCServiceAdapterBase::send(LocalIPCMessage &message)
{
    emit messageSent(message);
}

void LocalIPCServiceAdapterBase::sendReply(LocalIPCMessage &message)
{
    message.notifyListener();
}

}

}
