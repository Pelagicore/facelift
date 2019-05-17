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

#include "LocalIPC.h"

#include <QObject>
#include <QDebug>
#include <QTextStream>
#include <QTimer>

#include "FaceliftModel.h"
#include "FaceliftUtils.h"
#include "FaceliftProperty.h"

#include "QMLAdapter.h"
#include "QMLModel.h"

#include "LocalIPCProxy.h"
#include "LocalIPC-serialization.h"
#include "LocalIPCRegistry.h"


namespace facelift {
namespace local {


struct FaceliftIPCLocalLib_EXPORT FaceliftIPCCommon
{
    static constexpr const char *GET_PROPERTIES_MESSAGE_NAME = "GetAllProperties";
    static constexpr const char *PROPERTIES_CHANGED_SIGNAL_NAME = "PropertiesChanged";
    static constexpr const char *SIGNAL_TRIGGERED_SIGNAL_NAME = "SignalTriggered";
    static constexpr const char *SET_PROPERTY_MESSAGE_NAME = "SetProperty";
};

constexpr const char *FaceliftIPCCommon::SIGNAL_TRIGGERED_SIGNAL_NAME;


void LocalIPCServiceAdapterBase::initOutgoingSignalMessage()
{
    m_pendingOutgoingMessage = std::make_unique<LocalIPCMessage>(FaceliftIPCCommon::SIGNAL_TRIGGERED_SIGNAL_NAME);

    // Send property value updates before the signal itself so that they are set before the signal is triggered on the client side.
    this->serializePropertyValues(*m_pendingOutgoingMessage, false);
}

void LocalIPCServiceAdapterBase::serializePropertyValues(LocalIPCMessage &msg, bool isCompleteSnapshot)
{
    Q_UNUSED(isCompleteSnapshot);
    Q_ASSERT(service());
    serializeValue(msg, service()->ready());
}

void LocalIPCServiceAdapterBase::flush()
{
    if (m_pendingOutgoingMessage) {
        this->send(*m_pendingOutgoingMessage);
        m_pendingOutgoingMessage.reset();
    }
}

IPCHandlingResult LocalIPCServiceAdapterBase::handleMessage(LocalIPCMessage &requestMessage)
{
    LocalIPCMessage replyMessage = requestMessage.createReply();

    qCDebug(LogIpc) << "Handling incoming message: " << requestMessage.toString();

    auto handlingResult = IPCHandlingResult::OK;

    bool sendReply = true;
    if (requestMessage.member() == FaceliftIPCCommon::GET_PROPERTIES_MESSAGE_NAME) {
        serializePropertyValues(replyMessage, true);
    } else {
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

//LocalIPCManager &LocalIPCServiceAdapterBase::manager()
//{
//    return LocalIPCManager::instance();
//}

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
            QObject::connect(service(), &InterfaceBase::readyChanged, this, [this]() {
                        this->sendSignal(CommonSignalID::readyChanged);
                    });
            connectSignals();
        } else {
            qFatal("Could not register service at object path '%s'", qPrintable(objectPath()));
        }
    }
}

LocalIPCProxyBinder::LocalIPCProxyBinder(InterfaceBase &owner, QObject *parent) :
    IPCProxyBinderBase(owner, parent)
{
}


void LocalIPCProxyBinder::onServiceAvailable(LocalIPCServiceAdapterBase *adapter)
{

    Q_ASSERT(adapter != nullptr);
    if (m_serviceAdapter) {
        QObject::disconnect(m_signalConnection);
    }
    if (!isServiceAvailable()) {
        m_serviceAdapter = adapter;
        m_signalConnection = QObject::connect(adapter, &LocalIPCServiceAdapterBase::messageSent, this, [this] (LocalIPCMessage &message) {
                    this->onSignalTriggered(message);
                });
        requestPropertyValues();
        emit serviceAvailableChanged();
    }
}

void LocalIPCProxyBinder::onServiceUnavailable()
{
    if (isServiceAvailable()) {
        m_serviceAdapter = nullptr;
        emit serviceAvailableChanged();
    }
}

void LocalIPCProxyBinder::notifyServiceAvailable()
{
    serviceAvailableChanged();
}

void LocalIPCProxyBinder::setServiceAvailable(bool isRegistered)
{
    Q_UNUSED(isRegistered)

    auto isAvailable = isServiceAvailable();
    if (m_serviceAvailable != isAvailable) {
        m_serviceAvailable = isAvailable;
        emit serviceAvailableChanged();
    }
}

void LocalIPCProxyBinder::setServiceName(const QString &name)
{
    m_serviceName = name;
    m_explicitServiceName = true;
    checkInit();
}

void LocalIPCProxyBinder::setInterfaceName(const QString &name)
{
    m_interfaceName = name;
    checkInit();
}

void LocalIPCProxyBinder::onServerNotAvailableError(const char *methodName) const
{
    qCCritical(LogIpc,
            "Error message received when calling method '%s' on service at path '%s'. "
            "This likely indicates that the server you are trying to access is not available yet",
            qPrintable(methodName), qPrintable(objectPath()));
}

void LocalIPCProxyBinder::onPropertiesChanged(LocalIPCMessage &msg)
{
    m_serviceObject->deserializePropertyValues(msg, false);
}

void LocalIPCProxyBinder::onSignalTriggered(LocalIPCMessage &msg)
{
    m_serviceObject->deserializePropertyValues(msg, false);
    m_serviceObject->deserializeSignal(msg);
}

LocalIPCMessage LocalIPCProxyBinder::call(LocalIPCMessage &message) const
{
    //    return M_UNIMPLEMENTED<LocalIPCMessage>(message);
    Q_UNUSED(message);
    qFatal("Local IPC only used with Async proxies for now");
    return LocalIPCMessage();
}


void LocalIPCProxyBinder::asyncCall(LocalIPCMessage &requestMessage, QObject *context, std::function<void(LocalIPCMessage &message)> callback)
{
    requestMessage.addListener(context, callback);
    QTimer::singleShot(0, context, [this, requestMessage, callback]() mutable {
                auto r = m_serviceAdapter->handleMessage(requestMessage);
                Q_ASSERT(r != facelift::IPCHandlingResult::INVALID);
            });
}

void LocalIPCProxyBinder::requestPropertyValues()
{
    LocalIPCMessage msg(FaceliftIPCCommon::GET_PROPERTIES_MESSAGE_NAME);

    auto replyHandler = [this](LocalIPCMessage &replyMessage) {
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

void LocalIPCProxyBinder::bindToIPC()
{
    QObject::connect(&LocalIPCRegistry::instance(), &LocalIPCRegistry::adapterAvailable, this, [this](LocalIPCServiceAdapterBase *adapter) {
                if (adapter->objectPath() == this->objectPath()) {
                    onServiceAvailable(adapter);
                }
            });

    QObject::connect(&LocalIPCRegistry::instance(), &LocalIPCRegistry::adapterUnavailable, this,
            [this](const QString &objectPath, LocalIPCServiceAdapterBase *adapter) {
                Q_UNUSED(adapter);
                if (objectPath == this->objectPath()) {
                    onServiceUnavailable();
                }
            });

    auto serviceAdapter = LocalIPCRegistry::instance().getAdapter(objectPath());
    if (serviceAdapter) {
        onServiceAvailable(serviceAdapter);
    }

}

QString LocalIPCMessage::toString() const
{
    QString str;
    QTextStream s(&str);

    s << "Local IPC message ";
    s << " member:" << m_data.m_member;
    s << m_data.m_payload;
    return str;
}

OutputPayLoad &LocalIPCMessage::outputPayLoad()
{
    if (m_outputPayload == nullptr) {
        m_outputPayload = std::make_unique<OutputPayLoad>(m_data.m_payload);
    }
    return *m_outputPayload;
}

InputPayLoad &LocalIPCMessage::inputPayLoad()
{
    if (m_inputPayload == nullptr) {
        m_inputPayload = std::make_unique<InputPayLoad>(m_data.m_payload);
    }
    return *m_inputPayload;
}


void LocalIPCRegistry::registerAdapter(const QString &objectPath, LocalIPCServiceAdapterBase *adapter)
{
    Q_ASSERT(adapter);
    if ((!m_registry.contains(objectPath)) || (m_registry[objectPath] == nullptr)) {
        m_registry.insert(objectPath, adapter);
        emit adapterAvailable(adapter);
    } else {
        qFatal("Can't register new object at path: '%s'. Previously registered object: %s", qPrintable(objectPath),
                qPrintable(facelift::toString(*m_registry[objectPath]->service())));
    }
}

void LocalIPCRegistry::unregisterAdapter(LocalIPCServiceAdapterBase *adapter)
{
    for (auto &key : m_registry.keys()) {
        if (m_registry[key] == adapter) {
            m_registry.remove(key);
            emit adapterUnavailable(key, adapter);
            qCDebug(LogIpc) << "IPC service unregistered" << adapter;
            break;
        }
    }
}

InterfaceBase *LocalIPCRegistry::serviceMatches(const QString &objectPath, LocalIPCServiceAdapterBase *adapter)
{
    if (adapter->objectPath() == objectPath) {
        return adapter->service();
    } else {
        return nullptr;
    }
}

LocalIPCServiceAdapterBase *LocalIPCRegistry::getAdapter(const QString &objectPath)
{
    if (m_registry.contains(objectPath)) {
        return m_registry[objectPath];
    } else {
        return nullptr;
    }
}

LocalIPCRegistry &LocalIPCRegistry::instance()
{
    static LocalIPCRegistry registry;
    return registry;
}

}

}
