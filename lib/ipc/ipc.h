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
#include <QDBusServiceWatcher>

#include "Model.h"
#include "utils.h"

#include "ipc-config.h"

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

    bool isReplyMessage() const
    {
        return (m_message.type() == QDBusMessage::ReplyMessage);
    }

private:
    QDBusMessage &msg()
    {
        return m_message;
    }

    QDBusMessage m_message;

    size_t m_readPos = 0;
};



template<typename Type, typename Enable = void>
struct IPCTypeHandler
{
    static void writeDBUSSignature(QTextStream &s)
    {
        s << "i";
    }

    static void write(IPCMessage &msg, const Type &v)
    {
        msg.writeSimple(v);
    }

    static void read(IPCMessage &msg, Type &v)
    {
        msg.readNextParameter(v);
    }

};


template<size_t I = 0, typename ... Ts>
typename std::enable_if<I == sizeof ... (Ts)>::type
appendDBUSTypeSignature(QTextStream &s, std::tuple<Ts ...> &t)
{
    Q_UNUSED(s);
    Q_UNUSED(t);
}

template<size_t I = 0, typename ... Ts>
typename std::enable_if < I<sizeof ... (Ts)>::type
appendDBUSTypeSignature(QTextStream &s, std::tuple<Ts ...> &t)
{
    typedef typeof (std::get<I>(t)) Type;
    IPCTypeHandler<Type>::writeDBUSSignature(s);
    appendDBUSTypeSignature<I + 1>(s, t);
}


template<size_t I = 0, typename ... Ts>
typename std::enable_if<I == sizeof ... (Ts)>::type
appendDBUSMethodArgumentsSignature(QTextStream &s, std::tuple<Ts ...> &t, const std::array<const char *,
        sizeof ... (Ts)> &argNames)
{
    Q_UNUSED(s);
    Q_UNUSED(t);
    Q_UNUSED(argNames);
}

template<size_t I = 0, typename ... Ts>
typename std::enable_if < I<sizeof ... (Ts)>::type
appendDBUSMethodArgumentsSignature(QTextStream &s, std::tuple<Ts ...> &t, const std::array<const char *,
        sizeof ... (Ts)> &argNames)
{
    typedef typeof (std::get<I>(t)) Type;
    s << "<arg name=\"" << argNames[I] << "\" type=\"";
    IPCTypeHandler<Type>::writeDBUSSignature(s);
    s << "\" direction=\"in\"/>";
    appendDBUSMethodArgumentsSignature<I + 1>(s, t, argNames);
}


template<size_t I = 0, typename ... Ts>
typename std::enable_if<I == sizeof ... (Ts)>::type
appendDBUSSignalArgumentsSignature(QTextStream &s, std::tuple<Ts ...> &t, const std::array<const char *,
        sizeof ... (Ts)> &argNames)
{
    Q_UNUSED(s);
    Q_UNUSED(t);
    Q_UNUSED(argNames);
}

template<size_t I = 0, typename ... Ts>
typename std::enable_if < I<sizeof ... (Ts)>::type
appendDBUSSignalArgumentsSignature(QTextStream &s, std::tuple<Ts ...> &t, const std::array<const char *,
        sizeof ... (Ts)> &argNames)
{
    typedef typeof (std::get<I>(t)) Type;
    s << "<arg name=\"" << argNames[I] << "\" type=\"";
    IPCTypeHandler<Type>::writeDBUSSignature(s);
    s << "\"/>";
    appendDBUSSignalArgumentsSignature<I + 1>(s, t, argNames);
}


struct AppendDBUSSignatureFunction
{
    AppendDBUSSignatureFunction(QTextStream &s) :
        s(s)
    {
    }

    QTextStream &s;

    template<typename T>
    void operator()(T &&t)
    {
        Q_UNUSED(t);
        typedef typename std::decay<T>::type TupleType;
        std::tuple<TupleType> dummyTuple;
        appendDBUSTypeSignature(s, dummyTuple);
    }
};

template<>
struct IPCTypeHandler<float>
{
    static void writeDBUSSignature(QTextStream &s)
    {
        s << "d";
    }

    static void write(IPCMessage &msg, const float &v)
    {
        msg.writeSimple((double)v);
    }

    static void read(IPCMessage &msg, float &v)
    {
        double d;
        msg.readNextParameter(d);
        v = d;
    }

};


template<>
struct IPCTypeHandler<bool>
{
    static void writeDBUSSignature(QTextStream &s)
    {
        s << "b";
    }

    static void write(IPCMessage &msg, const bool &v)
    {
        msg.writeSimple(v);
    }

    static void read(IPCMessage &msg, bool &v)
    {
        msg.readNextParameter(v);
    }

};


template<>
struct IPCTypeHandler<QString>
{
    static void writeDBUSSignature(QTextStream &s)
    {
        s << "s";
    }

    static void write(IPCMessage &msg, const QString &v)
    {
        msg.writeSimple(v);
    }

    static void read(IPCMessage &msg, QString &v)
    {
        msg.readNextParameter(v);
    }

};

template<typename Type>
struct IPCTypeHandler<Type, typename std::enable_if<std::is_base_of<ModelStructure, Type>::value>::type>
{

    static void writeDBUSSignature(QTextStream &s)
    {
        typename Type::FieldTupleTypes t;          // TODO : get rid of that tuple
        s << "(";
        for_each_in_tuple(t, AppendDBUSSignatureFunction(s));
        s << ")";
    }

    static void write(IPCMessage &msg, const Type &param)
    {
        for_each_in_tuple_const(param.asTuple(), StreamWriteFunction<IPCMessage>(msg));
        param.id();
        msg << param.id();
    }

    static void read(IPCMessage &msg, Type &param)
    {
        typename Type::FieldTupleTypes tuple;
        for_each_in_tuple(tuple, StreamReadFunction<IPCMessage>(msg));
        param.setValue(tuple);
        ModelElementID id;
        msg.readNextParameter(id);
        param.setId(id);
    }

};

template<typename Type>
struct IPCTypeHandler<Type, typename std::enable_if<std::is_enum<Type>::value>::type>
{
    static void writeDBUSSignature(QTextStream &s)
    {
        s << "i";
    }

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
    static void writeDBUSSignature(QTextStream &s)
    {
        s << "a";
        IPCTypeHandler<ElementType>::writeDBUSSignature(s);
    }

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


enum class IPCHandlingResult {
    OK,
    INVALID
};


class DBusManager
{

public:
    DBusManager();

    static DBusManager &instance()
    {
        static DBusManager i;
        return i;
    }

    bool isDBusConnected() const
    {
        return m_dbusConnected;
    }

    bool registerServiceName(const QString &serviceName)
    {
        auto success = m_busConnection.registerService(serviceName);
        Q_ASSERT(success);
        return success;
    }

    QDBusConnection &connection()
    {
        return m_busConnection;
    }

private:
    QDBusConnection m_busConnection;
    bool m_dbusConnected;
};

class IPCServiceAdapterBase;

class InterfaceManager :
    public QObject
{
    Q_OBJECT

public:
    void registerAdapter(QString id, IPCServiceAdapterBase &object);

    IPCServiceAdapterBase *getAdapter(QString id);

    void onObjectDestroyed(QObject *object);

    Q_SIGNAL void adapterDestroyed(IPCServiceAdapterBase *adapter);
    Q_SIGNAL void adapterAvailable(IPCServiceAdapterBase *adapter);

    static InterfaceManager &instance()
    {
        static InterfaceManager registry;
        return registry;
    }

private:
    QMap<QString, IPCServiceAdapterBase *> m_registry;

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
    static constexpr const char *INTROSPECTABLE_INTERFACE_NAME = "org.freedesktop.DBus.Introspectable";
    static constexpr const char *PROPERTIES_INTERFACE_NAME = "org.freedesktop.DBus.Properties";

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
        QDBusVirtualObject(parent)
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

    void onPropertyValueChanged()
    {
        IPCMessage msg(m_objectPath, m_interfaceName, PROPERTIES_CHANGED_SIGNAL_NAME);
        serializePropertyValues(msg);
        msg.send(dbusManager().connection());
    }

    template<typename ... Args>
    void sendSignal(const char *signalName, const Args & ... args)
    {
        IPCMessage msg(m_objectPath, m_interfaceName, SIGNAL_TRIGGERED_SIGNAL_NAME);
        msg << signalName;
        auto argTuple = std::make_tuple(args ...);
        for_each_in_tuple(argTuple, StreamWriteFunction<IPCMessage>(msg));
        msg.send(dbusManager().connection());
    }

    template<typename Type>
    void addPropertySignature(QTextStream &s, const char *propertyName) const
    {
        s << "<property name=\"" << propertyName << "\" type=\"";
        std::tuple<Type> dummyTuple;
        appendDBUSTypeSignature(s, dummyTuple);
        s << "\" access=\"read\"/>";
    }

    template<typename ... Args>
    void addMethodSignature(QTextStream &s, const char *methodName,
            const std::array<const char *, sizeof ... (Args)> &argNames) const
    {
        s << "<method name=\"" << methodName << "\">";
        std::tuple<Args ...> t;  // TODO : get rid of the tuple
        appendDBUSMethodArgumentsSignature(s, t, argNames);
        s << "</method>";
    }

    template<typename ... Args>
    void addSignalSignature(QTextStream &s, const char *methodName,
            const std::array<const char *, sizeof ... (Args)> &argNames) const
    {
        s << "<signal name=\"" << methodName << "\">";
        std::tuple<Args ...> t;  // TODO : get rid of the tuple
        appendDBUSSignalArgumentsSignature(s, t, argNames);
        s << "</signal>";
    }

    virtual void connectSignals() = 0;
    virtual IPCHandlingResult handleMethodCallMessage(IPCMessage &requestMessage, IPCMessage &replyMessage) = 0;
    virtual void serializePropertyValues(IPCMessage &replyMessage) = 0;

    void init(QObject *service)
    {
        //        qDebug() << "m_interfaceName:" << m_interfaceName << " objectPath:" << m_objectPath << " service:" << m_service;
        if (!m_alreadyInitialized && m_enabled) {
            if ((service != nullptr) && !m_interfaceName.isEmpty() && !m_objectPath.isEmpty()) {

                if (dbusManager().isDBusConnected()) {
                    qWarning() << "Registering serviceName " << m_serviceName;

                    auto success = DBusManager::instance().registerServiceName(m_serviceName);
                    Q_ASSERT(success);

                    qDebug() << "Registering IPC object at " << m_objectPath;
                    m_alreadyInitialized = dbusManager().connection().registerVirtualObject(m_objectPath, this);
                    if (m_alreadyInitialized) {
                        connectSignals();
                    } else {
                        qCritical() << "Could no register service at object path" << m_objectPath;
                    }
                }

                InterfaceManager::instance().registerAdapter(m_objectPath, *this);
            }
        }
    }

    DBusManager &dbusManager()
    {
        return DBusManager::instance();
    }

protected:
    bool m_enabled = true;

    QString m_interfaceName = "";
    QString m_objectPath = "";
    QString m_introspectionData;
    QString m_serviceName = DEFAULT_SERVICE_NAME;

    bool m_alreadyInitialized = false;

};



template<typename ServiceType>
class IPCServiceAdapter :
    public IPCServiceAdapterBase
{
public:
    typedef ServiceType TheServiceType;

    IPCServiceAdapter()
    {
        setInterfaceName(ServiceType::FULLY_QUALIFIED_INTERFACE_NAME);
    }

    ServiceType *service() const
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

    virtual void appendDBUSIntrospectionData(QTextStream &s) const = 0;

    void init() override
    {
        IPCServiceAdapterBase::init(m_service);
    }

    QString introspect(const QString &path) const
    {
        QString introspectionData;

        if (path == m_objectPath) {
            QTextStream s(&introspectionData);
            s << "<interface name=\"" << m_interfaceName << "\">";
            appendDBUSIntrospectionData(s);
            s << "</interface>";
        } else {
            qFatal("Wrong object path");
        }

        qDebug() << "Introspection data for " << path << ":" << introspectionData;
        return introspectionData;
    }

protected:
    QPointer<ServiceType> m_service;

};

class IPCRequestHandler
{

public:
    virtual ~IPCRequestHandler()
    {
    }

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

                if (manager().isDBusConnected()) {

                    m_busWatcher.addWatchedService(m_serviceName);
                    connect(&m_busWatcher, &QDBusServiceWatcher::serviceRegistered, this, &IPCProxyBinder::onServiceAvailable);

                    qWarning() << "Registering Proxy";
                    auto successPropertyChangeSignal =
                            connection().connect(m_serviceName, m_objectPath, m_interfaceName,
                                    IPCServiceAdapterBase::PROPERTIES_CHANGED_SIGNAL_NAME,
                                    this, SLOT(onPropertiesChanged(
                                        const QDBusMessage&)));
                    assert(successPropertyChangeSignal);

                    auto successSignalTriggeredSignal =
                            connection().connect(m_serviceName, m_objectPath, m_interfaceName,
                                    IPCServiceAdapterBase::SIGNAL_TRIGGERED_SIGNAL_NAME,
                                    this, SLOT(onSignalTriggered(
                                        const QDBusMessage&)));
                    assert(successSignalTriggeredSignal);

                    requestPropertyValues();

                }

                QObject::connect(
                    &InterfaceManager::instance(), &InterfaceManager::adapterAvailable, this,
                    &IPCProxyBinder::onLocalAdapterAvailable);

                m_alreadyInitialized = true;

            }
        }
    }

    void onLocalAdapterAvailable(IPCServiceAdapterBase *adapter)
    {
        if (adapter->objectPath() == this->objectPath()) {
            localAdapterAvailable(adapter);
        }
    }

    Q_SIGNAL void localAdapterAvailable(IPCServiceAdapterBase *adapter);

    void onServiceAvailable()
    {
        requestPropertyValues();
    }

    IPCProxyBinder(QObject *parent = nullptr) :
        QObject(parent)
    {
    }

    void requestPropertyValues()
    {
        IPCMessage msg(m_serviceName, m_objectPath, m_interfaceName,
                IPCServiceAdapterBase::GET_PROPERTIES_MESSAGE_NAME);
        auto replyMessage = msg.call(connection());
        if (replyMessage.isReplyMessage()) {
            m_serviceObject->deserializePropertyValues(replyMessage);
        } else {
            qWarning() << "Service not yet available";
        }
    }

    template<typename ... Args>
    IPCMessage sendMethodCall(const char *methodName, const Args & ... args)
    {
        IPCMessage msg(m_serviceName, m_objectPath, m_interfaceName, methodName);
        auto argTuple = std::make_tuple(args ...);
        for_each_in_tuple(argTuple, StreamWriteFunction<IPCMessage>(msg));
        auto replyMessage = msg.call(connection());
        if (replyMessage.isReplyMessage()) {
        } else {
            qWarning() << "Error message received";
            Q_ASSERT(false);
        }
        return replyMessage;
    }

    QDBusConnection &connection()
    {
        return manager().connection();
    }

    DBusManager &manager()
    {
        return DBusManager::instance();
    }

    void setHandler(IPCRequestHandler *handler)
    {
        m_serviceObject = handler;
        init();
    }

private:
    bool m_alreadyInitialized = false;
    bool m_enabled = true;

    QString m_interfaceName;
    QString m_objectPath;
    QString m_serviceName = IPCServiceAdapterBase::DEFAULT_SERVICE_NAME;

    IPCRequestHandler *m_serviceObject = nullptr;

    QDBusServiceWatcher m_busWatcher;
};


template<typename Type, typename IPCAdapterType>
class IPCProxy :
    public Type, protected IPCRequestHandler
{

public:
    typedef typename IPCAdapterType::TheServiceType InterfaceType;

    IPCProxy(QObject *parent = nullptr) :
        Type(parent)
    {
        m_ipcBinder.setInterfaceName(Type::FULLY_QUALIFIED_INTERFACE_NAME);
        m_ipcBinder.setHandler(this);

        QObject::connect(&m_ipcBinder, &IPCProxyBinder::localAdapterAvailable, this, &IPCProxy::onLocalAdapterAvailable);
    }

    void onLocalAdapterAvailable(IPCServiceAdapterBase *a)
    {
        auto adapter = qobject_cast<IPCAdapterType *>(a);
        if (adapter != nullptr) {
            m_localAdapter = adapter;
        }

        if (localInterface() != nullptr) {
            bindLocalService(localInterface());
        }
    }

    virtual void bindLocalService(InterfaceType *service) = 0;

    void loadPropertiesFrom(typename IPCAdapterType::TheServiceType &adapter)
    {
    }

    template<typename ... Args>
    void sendMethodCall(const char *methodName, const Args & ... args)
    {
        auto msg = m_ipcBinder.sendMethodCall(methodName, args ...);
    }

    template<typename ReturnType, typename ... Args>
    void sendMethodCallWithReturn(const char *methodName, ReturnType &returnValue, const Args & ... args)
    {
        auto msg = m_ipcBinder.sendMethodCall(methodName, args ...);
        if (msg.isReplyMessage()) {
            IPCTypeHandler<ReturnType>::read(msg, returnValue);
        }
    }

    IPCProxyBinder *ipc()
    {
        return &m_ipcBinder;
    }

    InterfaceType *localInterface()
    {
        if (m_localAdapter) {
            return m_localAdapter->service();
        } else {
            return nullptr;
        }
    }

private:
    QPointer<IPCAdapterType> m_localAdapter = nullptr;
    IPCProxyBinder m_ipcBinder;

};
