/*
 *   Copyright (C) 2017 Pelagicore AG
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 *   Please see the LICENSE file for details.
 */

#pragma once

#include "assert.h"

#include <QObject>
#include <QTextStream>
#include <QTimer>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusVirtualObject>
#include <QDBusAbstractInterface>
#include <QDBusInterface>

#include "Model.h"

class IPCMessage
{

public:
    IPCMessage() :
        IPCMessage("dummy/dummy", "dummy.dummy", "gg")
    {
    }

    IPCMessage(const QDBusMessage &msg)
    {
        m_message = msg;
    }

    IPCMessage(const QString &service, const QString &path, const QString &interface, const QString &method)
    {
        m_message = QDBusMessage::createMethodCall(service, path, interface, method);
    }

    IPCMessage(const QString &path, const QString &interface, const QString &signal)
    {
        m_message = QDBusMessage::createSignal(path, interface, signal);
    }

    IPCMessage call(const QDBusConnection &connection)
    {
        qDebug() << "Sending IPC message : " << toString();
        auto replyDbusMessage = connection.call(m_message);
        IPCMessage reply(replyDbusMessage);
        return reply;
    }

    void send(const QDBusConnection &connection)
    {
        qDebug() << "Sending IPC message : " << toString();
        bool successful = connection.send(m_message);
        assert(successful);
    }

    QString member() const
    {
        return m_message.member();
    }

    QString toString() const
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

    IPCMessage createReply()
    {
        return IPCMessage(m_message.createReply());
    }

    IPCMessage createErrorReply(const QString &msg, const QString &member)
    {
        return IPCMessage(m_message.createErrorReply(msg, member));
    }

    template<typename Type>
    void readNextParameter(Type &v)
    {
        auto asVariant = m_message.arguments()[m_readPos++];
        //        qDebug() << asVariant;
        v = asVariant.value<Type>();
    }

    template<typename Type>
    void writeSimple(const Type &v)
    {
        //        qDebug() << "Writing to message : " << v;
        msg() << v;
    }

    QString signature() const
    {
        return m_message.signature();
    }

private:
    QDBusMessage &msg()
    {
        return m_message;
    }

    QDBusMessage m_message;

    size_t m_readPos = 0;
};

struct StreamReadFunction
{
    StreamReadFunction(IPCMessage &msg) :
        m_msg(msg)
    {
    }

    IPCMessage &m_msg;

    template<typename T>
    void operator()(T &t)
    {
        m_msg >> t;
    }
};

struct StreamWriteFunction
{
    StreamWriteFunction(IPCMessage &msg) :
        m_msg(msg)
    {
    }

    IPCMessage &m_msg;

    template<typename T>
    void operator()(T && t)
    {
        m_msg << t;
    }
};

template<size_t I = 0, typename Func, typename ... Ts>
typename std::enable_if<I == sizeof ... (Ts)>::type
for_each_in_tuple(std::tuple<Ts ...> &, Func)
{
}

template<size_t I = 0, typename Func, typename ... Ts>
typename std::enable_if < I<sizeof ... (Ts)>::type
for_each_in_tuple(std::tuple<Ts ...> &tpl, Func func)
{
    func(std::get<I>(tpl));
    for_each_in_tuple<I + 1>(tpl, func);
}


template<typename Type, typename Enable = void>
struct IPCTypeHandler
{

    static void write(IPCMessage &msg, const Type &v)
    {
        msg.writeSimple(v);
    }

    static void read(IPCMessage &msg, Type &v)
    {
        msg.readNextParameter(v);
    }

};

template<typename Type>
struct IPCTypeHandler<Type, typename std::enable_if<std::is_base_of<ModelStructure, Type>::value>::type>
{

    static void write(IPCMessage &msg, const Type &param)
    {
        auto tupleCopy = param.asTuple();
        for_each_in_tuple(tupleCopy, StreamWriteFunction(msg));
        param.id();
        msg << param.id();
    }

    static void read(IPCMessage &msg, Type &param)
    {
        typename Type::FieldTupleTypes tuple;
        for_each_in_tuple(tuple, StreamReadFunction(msg));
        param.setValue(tuple);
        ModelElementID id;
        msg.readNextParameter(id);
        param.setId(id);
    }

};

template<typename Type>
struct IPCTypeHandler<Type, typename std::enable_if<std::is_enum<Type>::value>::type>
{

    static void write(IPCMessage &msg, const Type &param)
    {
        msg.writeSimple(static_cast<int>(param));
    }

    static void read(IPCMessage &msg, Type &param)
    {
        int i;
        msg.readNextParameter(i);
        param = static_cast<Type>(i);
    }
};


template<typename ElementType>
struct IPCTypeHandler<QList<ElementType> >
{

    static void write(IPCMessage &msg, const QList<ElementType> &list)
    {
        int count = list.size();
        msg.writeSimple(count);
        for (const auto &e : list) {
            IPCTypeHandler<ElementType>::write(msg, e);
        }
    }

    static void read(IPCMessage &msg, QList<ElementType> &list)
    {
        list.clear();
        int count;
        msg.readNextParameter(count);
        for (int i = 0; i < count; i++) {
            ElementType e;
            IPCTypeHandler<ElementType>::read(msg, e);
            list.append(e);
        }
    }

};


template<typename Type>
IPCMessage &operator<<(IPCMessage &msg, const Type &v)
{
    IPCTypeHandler<Type>::write(msg, v);
    return msg;
}


template<typename Type>
IPCMessage &operator>>(IPCMessage &msg, Type &v)
{
    IPCTypeHandler<Type>::read(msg, v);
    return msg;
}


template<typename Type>
static QString getTypeSignature()
{
    IPCMessage msg;
    Type t = {};
    msg << t;
    qDebug() << msg.toString();
    return msg.signature();
}

/*
struct AppendSignatureFunction {

    AppendSignatureFunction(QString& signature) : m_signature(signature) {
    }

    template<typename T>
    void operator ()(T& t) {
        IPCTypeHandler<T>::appendSignature(m_signature);
    }

    QString& m_signature;
};


template<typename Type> QString generateDBusSignature() {
    QString signature;
//    IPCTypeHandler<Type>::appendSignature(signature);
    return signature;
}
*/

enum class IPCHandlingResult
{
    OK,
    INVALID
};

class IPCServiceAdapterBase :
    public QDBusVirtualObject
{

    Q_OBJECT

public:
    static constexpr const char *DEFAULT_SERVICE_NAME = "qface.ipc";

    static constexpr const char *GET_PROPERTIES_MESSAGE_NAME = "GetAllProperties";
    static constexpr const char *PROPERTIES_CHANGED_SIGNAL_NAME = "PropertiesChanged";
    static constexpr const char *SIGNAL_TRIGGERED_SIGNAL_NAME = "SignalTriggered";
    static constexpr const char *SET_PROPERTY_MESSAGE_NAME = "SetProperty";
    static constexpr const char *INTROSPECT_MESSAGE_NAME = "org.freedesktop.DBus.Introspectable";

    Q_PROPERTY(QObject * service READ service WRITE setService)
    Q_PROPERTY(QString objectPath READ objectPath WRITE setObjectPath)
    Q_PROPERTY(QString interfaceName READ interfaceName WRITE setInterfaceName)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)

    bool enabled() const
    {
        return m_enabled;
    }

    void setEnabled(bool enabled)
    {
        m_enabled = enabled;
        init();
    }

    virtual QObject *service() const = 0;

    virtual void setService(QObject *service) = 0;

    IPCServiceAdapterBase(QObject *parent = nullptr) :
        QDBusVirtualObject(parent), m_busConnection(QDBusConnection::sessionBus())
    {
    }

    const QString &objectPath() const
    {
        return m_objectPath;
    }

    void setObjectPath(const QString &objectPath)
    {
        m_objectPath = objectPath;
        init();
    }

    const QString &interfaceName() const
    {
        return m_interfaceName;
    }

    void setInterfaceName(const QString &name)
    {
        m_interfaceName = name;
        init();
    }

    virtual void init() = 0;

    bool handleMessage(const QDBusMessage &dbusMsg, const QDBusConnection &connection)
    {
        IPCMessage requestMessage(dbusMsg);

        IPCMessage replyMessage = requestMessage.createReply();

        qDebug() << "Handling incoming message: " << requestMessage.toString();

        if (dbusMsg.interface() != INTROSPECT_MESSAGE_NAME) {
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

    void onPropertyValueChanged()
    {
        IPCMessage msg(m_objectPath, m_interfaceName, PROPERTIES_CHANGED_SIGNAL_NAME);
        serializePropertyValues(msg);
        msg.send(m_busConnection);
    }

    template<typename ... Args>
    void sendSignal(const char *signalName, const Args & ... args)
    {
        IPCMessage msg(m_objectPath, m_interfaceName, SIGNAL_TRIGGERED_SIGNAL_NAME);
        msg << signalName;
        auto argTuple = std::make_tuple(args ...);
        for_each_in_tuple(argTuple, StreamWriteFunction(msg));
        msg.send(m_busConnection);
    }

    template<typename Type>
    void addPropertySignature(QTextStream &s, const char *propertyName) const
    {
        qDebug();
        s << "signature " << getTypeSignature<Type>() << " " << propertyName;
        qDebug();
    }

    template<typename ... Args>
    void addMethodSignature(QTextStream &s, const char *methodName) const
    {

        s << "<method name=\"" << methodName << "\">";

        qDebug() << "Signature " << getTypeSignature<int>();

        s << "</method>";
        qDebug();
    }

    virtual void connectSignals() = 0;
    virtual IPCHandlingResult handleMethodCallMessage(IPCMessage &requestMessage, IPCMessage &replyMessage) = 0;
    virtual void serializePropertyValues(IPCMessage &replyMessage) = 0;

protected:
    bool m_enabled = true;

    QDBusConnection m_busConnection;
    QString m_interfaceName = "";
    QString m_objectPath = "";
    QString m_introspectionData;

    QString m_serviceName = DEFAULT_SERVICE_NAME;
};


template<typename ServiceType>
class IPCServiceAdapter :
    public IPCServiceAdapterBase
{

public:
    IPCServiceAdapter()
    {
        setInterfaceName(ServiceType::IPC_INTERFACE_NAME);
    }

    QObject *service() const
    {
        return m_service;
    }

    void setService(QObject *service)
    {
        m_service = qobject_cast<ServiceType *>(service);
        if (m_service == nullptr) {
            typedef typename ServiceType::QMLFrontendType QMLFrontendType;
            auto *qmlFrontend = qobject_cast<QMLFrontendType *>(service);
            if (qmlFrontend != nullptr) {
                m_service = qmlFrontend->m_provider;
            } else {
                qWarning() << "Bad service type : " << service;
            }
        }
        Q_ASSERT(m_service != nullptr);

        init();
    }

    virtual void introspect(QTextStream &s) const = 0;

    QString introspect(const QString &path) const
    {
        QString a;
        QTextStream s(&a);
        qDebug() << "PPPPPP " << path;
        s << "<interface name=\"" << "org.qtproject.QtDBus.MyObject" << "\">";
        introspect(s);
        s << "</interface>";
        qDebug() << " " << a;
        return a;
    }

    void init()
    {
        //        qDebug() << "m_interfaceName:" << m_interfaceName << " objectPath:" << m_objectPath << " service:" << m_service;
        if (!m_alreadyInitialized && m_enabled) {
            if ((m_service != nullptr) && !m_interfaceName.isEmpty() && !m_objectPath.isEmpty()) {

                qWarning() << "Registering serviceName " << m_serviceName;
                auto success = m_busConnection.registerService(m_serviceName);
                Q_ASSERT(success);

                m_alreadyInitialized = m_busConnection.registerVirtualObject(m_objectPath, this);
                qWarning() << "Registering IPC object at " << m_objectPath << " successful: " << m_alreadyInitialized;
                Q_ASSERT(m_alreadyInitialized);
                connectSignals();
            }
        }
    }

protected:
    ServiceType *m_service = nullptr;
    bool m_alreadyInitialized = false;

};

class IPCRequestHandler
{

public:
    virtual void deserializePropertyValues(IPCMessage &msg) = 0;
    virtual void deserializeSignal(IPCMessage &msg) = 0;

};

class IPCProxyBinder :
    public QObject
{

    Q_OBJECT

public:
    Q_PROPERTY(QString objectPath READ objectPath WRITE setObjectPath)
    Q_PROPERTY(QString interfaceName READ interfaceName WRITE setInterfaceName)
    Q_PROPERTY(QString serviceName READ serviceName WRITE setServiceName)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)

    bool enabled() const
    {
        return m_enabled;
    }

    void setEnabled(bool enabled)
    {
        m_enabled = enabled;
        init();
    }

    Q_SLOT
    void onPropertiesChanged(const QDBusMessage &dbusMessage)
    {
        IPCMessage msg(dbusMessage);
        m_serviceObject->deserializePropertyValues(msg);
    }

    Q_SLOT
    void onSignalTriggered(const QDBusMessage &dbusMessage)
    {
        IPCMessage msg(dbusMessage);
        m_serviceObject->deserializeSignal(msg);
    }

    const QString &objectPath() const
    {
        return m_objectPath;
    }

    void setObjectPath(const QString &objectPath)
    {
        m_objectPath = objectPath;
        init();
    }

    const QString &serviceName() const
    {
        return m_serviceName;
    }

    void setServiceName(const QString &name)
    {
        m_serviceName = name;
        init();
    }

    const QString &interfaceName() const
    {
        return m_interfaceName;
    }

    void setInterfaceName(const QString &name)
    {
        m_interfaceName = name;
        init();
    }

    void init()
    {
        if (!m_alreadyInitialized && m_enabled) {
            if ((m_serviceName != nullptr) && !m_interfaceName.isEmpty() && !m_objectPath.isEmpty()) {

                qWarning() << "Registering Proxy";
                auto successPropertyChangeSignal =
                        bus().connect(m_serviceName, m_objectPath, m_interfaceName,
                        IPCServiceAdapterBase::PROPERTIES_CHANGED_SIGNAL_NAME,
                        this, SLOT(onPropertiesChanged(
                                const QDBusMessage &)));
                assert(successPropertyChangeSignal);

                auto successSignalTriggeredSignal =
                        bus().connect(m_serviceName, m_objectPath, m_interfaceName,
                        IPCServiceAdapterBase::SIGNAL_TRIGGERED_SIGNAL_NAME,
                        this, SLOT(onSignalTriggered(
                                const QDBusMessage &)));
                assert(successSignalTriggeredSignal);

                requestPropertyValues();

                m_alreadyInitialized = true;
            }
        }
    }

    IPCProxyBinder(QObject *parent = nullptr) :
        QObject(parent), m_busConnection(QDBusConnection::sessionBus())
    {
    }

    void requestPropertyValues()
    {
        IPCMessage msg(m_serviceName, m_objectPath, m_interfaceName,
                IPCServiceAdapterBase::GET_PROPERTIES_MESSAGE_NAME);
        auto replyMessage = msg.call(m_busConnection);
        m_serviceObject->deserializePropertyValues(replyMessage);
    }

    template<typename ... Args>
    IPCMessage sendMethodCall(const char *methodName, const Args & ... args)
    {
        IPCMessage msg(m_serviceName, m_objectPath, m_interfaceName, methodName);
        auto argTuple = std::make_tuple(args ...);
        for_each_in_tuple(argTuple, StreamWriteFunction(msg));
        return msg.call(m_busConnection);
    }

    QDBusConnection &bus()
    {
        return m_busConnection;
    }

    void setHandler(IPCRequestHandler *handler)
    {
        m_serviceObject = handler;
        init();
    }

private:
    bool m_alreadyInitialized = false;
    bool m_enabled = true;

    QDBusConnection m_busConnection;
    QString m_interfaceName;
    QString m_objectPath;
    QString m_serviceName = IPCServiceAdapterBase::DEFAULT_SERVICE_NAME;

    IPCRequestHandler *m_serviceObject = nullptr;

};


template<typename Type>
class IPCProxy :
    public Type, protected IPCRequestHandler
{

public:
    IPCProxy(QObject *parent = nullptr) :
        Type(parent)
    {
        m_ipcBinder.setInterfaceName(Type::IPC_INTERFACE_NAME);
        m_ipcBinder.setHandler(this);
    }

    template<typename ... Args>
    IPCMessage sendMethodCall(const char *methodName, const Args & ... args)
    {
        return m_ipcBinder.sendMethodCall(methodName, args ...);
    }

    IPCProxyBinder *ipc()
    {
        return &m_ipcBinder;
    }

private:
    IPCProxyBinder m_ipcBinder;

};
