/*
 *   Copyright (C) 2017 Pelagicore AG
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 *   Please see the LICENSE file for details.
 */

#pragma once

#include "assert.h"

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

#include "Model.h"
#include "utils.h"
#include "Property.h"

#include "QMLFrontend.h"

namespace facelift {

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
        Q_ASSERT(successful);
    }

    QString member() const
    {
        return m_message.member();
    }

    QString toString() const;

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

    bool isErrorMessage() const
    {
        return (m_message.type() == QDBusMessage::ErrorMessage);
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
struct IPCTypeHandler<Type, typename std::enable_if<std::is_base_of<StructureBase, Type>::value>::type>
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


template<typename Type>
IPCMessage &operator>>(IPCMessage &msg, Property<Type> &property)
{
    Type v;
    IPCTypeHandler<Type>::read(msg, v);
    property.setValue(v);
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

    static DBusManager &instance();

    bool isDBusConnected() const
    {
        return m_dbusConnected;
    }

    void registerServiceName(const QString &serviceName)
    {
        qDebug() << "Registering serviceName " << serviceName;
        auto success = m_busConnection.registerService(serviceName);
        Q_ASSERT(success);
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


class InterfaceManager : public QObject
{
    Q_OBJECT

public:
    void registerAdapter(QString id, IPCServiceAdapterBase &object);

    IPCServiceAdapterBase *getAdapter(QString id);

    void onAdapterDestroyed(IPCServiceAdapterBase *object);

    Q_SIGNAL void adapterDestroyed(IPCServiceAdapterBase *adapter);
    Q_SIGNAL void adapterAvailable(IPCServiceAdapterBase *adapter);

    static InterfaceManager &instance();

private:
    QMap<QString, IPCServiceAdapterBase *> m_registry;

};

class IPCServiceAdapterBase : public QDBusVirtualObject, public IPCAdapterBase
{

    Q_OBJECT

public:
    static constexpr const char *DEFAULT_SERVICE_NAME = "facelift.ipc";

    static constexpr const char *GET_PROPERTIES_MESSAGE_NAME = "GetAllProperties";
    static constexpr const char *PROPERTIES_CHANGED_SIGNAL_NAME = "PropertiesChanged";
    static constexpr const char *SIGNAL_TRIGGERED_SIGNAL_NAME = "SignalTriggered";
    static constexpr const char *SET_PROPERTY_MESSAGE_NAME = "SetProperty";
    static constexpr const char *INTROSPECTABLE_INTERFACE_NAME = "org.freedesktop.DBus.Introspectable";
    static constexpr const char *PROPERTIES_INTERFACE_NAME = "org.freedesktop.DBus.Properties";

    Q_PROPERTY(facelift::InterfaceBase * service READ service WRITE setService)
    Q_PROPERTY(QString objectPath READ objectPath WRITE setObjectPath)
    Q_PROPERTY(QString interfaceName READ interfaceName WRITE setInterfaceName)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)

    Q_SIGNAL void destroyed(IPCServiceAdapterBase *adapter);

    ~IPCServiceAdapterBase()
    {
        destroyed(this);
    }

    bool enabled() const
    {
        return m_enabled;
    }

    void onProviderCompleted()
    {
        // The parsing of the provide is finished => all our properties are set and we are ready to register our service
        m_complete = true;
        init();
    }

    void setEnabled(bool enabled)
    {
        m_enabled = enabled;
        init();
    }

    virtual InterfaceBase *service() const = 0;

    virtual void setService(InterfaceBase *service) = 0;

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

    bool handleMessage(const QDBusMessage &dbusMsg, const QDBusConnection &connection);

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
    void addPropertySignature(QTextStream &s, const char *propertyName, bool isReadonly) const
    {
        s << "<property name=\"" << propertyName << "\" type=\"";
        std::tuple<Type> dummyTuple;
        appendDBUSTypeSignature(s, dummyTuple);
        s << "\" access=\"" << (isReadonly ? "read" : "readwrite") << "\"/>";
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

    void serializePropertyValues(IPCMessage &replyMessage)
    {
        replyMessage << m_service->ready();
        serializeSpecificPropertyValues(replyMessage);
    }

    virtual void serializeSpecificPropertyValues(IPCMessage &replyMessage) = 0;

    void init(InterfaceBase *service);
    DBusManager &dbusManager()
    {
        return DBusManager::instance();
    }

protected:
    QString m_interfaceName = "";
    QString m_objectPath = "";
    QString m_introspectionData;
    QString m_serviceName = DEFAULT_SERVICE_NAME;

    InterfaceBase *m_service = nullptr;

    bool m_enabled = false;
    bool m_alreadyInitialized = false;
    bool m_complete = false;
};



template<typename ServiceType>
class IPCServiceAdapter : public IPCServiceAdapterBase
{
public:
    typedef ServiceType TheServiceType;

    IPCServiceAdapter(QObject *parent) : IPCServiceAdapterBase(parent)
    {
        setInterfaceName(ServiceType::FULLY_QUALIFIED_INTERFACE_NAME);
    }

    ServiceType *service() const override
    {
        return m_service;
    }

    void setService(InterfaceBase *service) override
    {
        m_service = qobject_cast<ServiceType *>(service);
        if (m_service == nullptr) {
            typedef typename ServiceType::QMLFrontendType QMLFrontendType;
            auto *qmlFrontend = qobject_cast<QMLFrontendType *>(service);
            if (qmlFrontend != nullptr) {
                m_service = qmlFrontend->m_provider;
            } else {
                qFatal("Bad service type : '%s'", qPrintable(facelift::toString(service)));
            }
        }
        Q_ASSERT(m_service != nullptr);
        QObject::connect(m_service, &InterfaceBase::componentCompleted, this, &IPCServiceAdapter::onProviderCompleted);
        init();
    }

    virtual void appendDBUSIntrospectionData(QTextStream &s) const = 0;

    void init() override final
    {
        IPCServiceAdapterBase::init(m_service);
    }

    QString introspect(const QString &path) const override
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
    virtual void setServiceRegistered(bool isRegistered) = 0;

};

class IPCProxyBinder : public QObject
{
    Q_OBJECT

public:
    Q_PROPERTY(QString objectPath READ objectPath WRITE setObjectPath)
    Q_PROPERTY(QString interfaceName READ interfaceName WRITE setInterfaceName)
    Q_PROPERTY(QString serviceName READ serviceName WRITE setServiceName)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)

    IPCProxyBinder(QObject *parent = nullptr) :
        QObject(parent)
    {
        m_busWatcher.setWatchMode(QDBusServiceWatcher::WatchForRegistration);
    }

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

    void onComponentCompleted()
    {
        m_componentCompleted = true;
        init();
    }

    void init();

    void onLocalAdapterAvailable(IPCServiceAdapterBase *adapter);

    Q_SIGNAL void localAdapterAvailable(IPCServiceAdapterBase *adapter);

    void onServiceAvailable()
    {
        requestPropertyValues();
    }

    void requestPropertyValues()
    {
        IPCMessage msg(m_serviceName, m_objectPath, m_interfaceName,
                IPCServiceAdapterBase::GET_PROPERTIES_MESSAGE_NAME);
        auto replyMessage = msg.call(connection());
        if (replyMessage.isReplyMessage()) {
            m_serviceObject->deserializePropertyValues(replyMessage);
            m_serviceObject->setServiceRegistered(true);
        } else {
            qDebug() << "Service not yet available : " << m_objectPath;
        }
    }

    template<typename PropertyType>
    void sendSetterCall(const char *methodName, const PropertyType &value)
    {
        IPCMessage msg(m_serviceName, m_objectPath, m_interfaceName, methodName);
        msg << value;
        auto replyMessage = msg.call(connection());
        if (replyMessage.isErrorMessage()) {
            qFatal("Error message received when calling method '%s' on service at path '%s'. This likely indicates that the server you are trying to access is not available yet"
                    , qPrintable(
                        methodName), qPrintable(m_objectPath));
        }
    }

    template<typename ... Args>
    IPCMessage sendMethodCall(const char *methodName, const Args & ... args)
    {
        IPCMessage msg(m_serviceName, m_objectPath, m_interfaceName, methodName);
        auto argTuple = std::make_tuple(args ...);
        for_each_in_tuple(argTuple, StreamWriteFunction<IPCMessage>(msg));
        auto replyMessage = msg.call(connection());
        if (replyMessage.isErrorMessage()) {
            qFatal("Error message received when calling method '%s' on service at path '%s'. This likely indicates that the server you are trying to access is not available yet"
                    , qPrintable(
                        methodName), qPrintable(m_objectPath));
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
    bool m_componentCompleted = false;

    IPCRequestHandler *m_serviceObject = nullptr;

    QDBusServiceWatcher m_busWatcher;
};


template<typename Type, typename IPCAdapterType>
class IPCProxy : public Type, protected IPCRequestHandler
{

public:
    typedef typename IPCAdapterType::TheServiceType InterfaceType;

    IPCProxy(QObject *parent = nullptr) :
        Type(parent)
    {
        m_ipcBinder.setInterfaceName(Type::FULLY_QUALIFIED_INTERFACE_NAME);
        m_ipcBinder.setHandler(this);

        m_serviceReady.init("ready", this, &InterfaceBase::readyChanged);

        QObject::connect(&m_ipcBinder, &IPCProxyBinder::localAdapterAvailable, this, &IPCProxy::onLocalAdapterAvailable);
        QObject::connect(this, &InterfaceBase::componentCompleted, &m_ipcBinder, &IPCProxyBinder::onComponentCompleted);

        this->setImplementationID("IPC Proxy");
    }

    virtual void deserializeSpecificPropertyValues(IPCMessage &msg) = 0;

    void deserializePropertyValues(IPCMessage &msg) override
    {
        msg >> m_serviceReady;
        deserializeSpecificPropertyValues(msg);
    }

    bool ready() const override
    {
        if (localInterface() != nullptr) {
            return localInterface()->ready();
        }
        return m_serviceReady;
    }

    void setServiceRegistered(bool isRegistered) override
    {
        bool oldReady = ready();
        m_serviceRegistered = isRegistered;
        if (ready() != oldReady) {
            this->readyChanged();
        }
    }

    void onLocalAdapterAvailable(IPCServiceAdapterBase *a)
    {
        auto adapter = qobject_cast<IPCAdapterType *>(a);
        if (adapter != nullptr) {
            m_localAdapter = adapter;
        }

        if (localInterface() != nullptr) {
            bindLocalService(localInterface());
            this->setReady(true);
        }
    }

    virtual void bindLocalService(InterfaceType *service) = 0;

    template<typename PropertyType>
    void sendSetterCall(const char *methodName, const PropertyType &value)
    {
        m_ipcBinder.sendSetterCall(methodName, value);
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

    InterfaceType *localInterface() const
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
    bool m_serviceRegistered = false;
    Property<bool> m_serviceReady;

};


class IPCAdapterFactoryManager
{
public:
    typedef IPCServiceAdapterBase * (*IPCAdapterFactory)(InterfaceBase *);

    static IPCAdapterFactoryManager &instance();

    template<typename AdapterType>
    static IPCServiceAdapterBase *createInstance(InterfaceBase *i)
    {
        auto adapter = new AdapterType(i);
        adapter->setService(i);
        qDebug() << "Created adapter for interface " << i->interfaceID();
        return adapter;
    }

    template<typename AdapterType>
    static void registerType()
    {
        auto &i = instance();
        const auto &typeID = AdapterType::TheServiceType::FULLY_QUALIFIED_INTERFACE_NAME;
        if (i.m_factories.contains(typeID)) {
            qWarning() << "IPC type already registered" << typeID;
        } else {
            i.m_factories.insert(typeID, &IPCAdapterFactoryManager::createInstance<AdapterType>);
        }
    }

    IPCAdapterFactory getFactory(const QString &typeID) const
    {
        if (m_factories.contains(typeID)) {
            return m_factories[typeID];
        } else {
            return nullptr;
        }
    }

private:
    QMap<QString, IPCAdapterFactory> m_factories;

};

class IPCAdapterAttachedType : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString objectPath READ objectPath WRITE setObjectPath NOTIFY objectPathChanged)
public:
    IPCAdapterAttachedType(QObject *parent) : QObject(parent)
    {
    }

    Q_SIGNAL void objectPathChanged();

    const QString &objectPath() const
    {
        return m_objectPath;
    }

    void setObjectPath(const QString &objectPath)
    {
        m_objectPath = objectPath;
    }

private:
    QString m_objectPath;

};


class IPCAttachedPropertyFactory : public QObject
{
    Q_OBJECT

public:
    static IPCServiceAdapterBase *qmlAttachedProperties(QObject *object);

};

}


QML_DECLARE_TYPEINFO(facelift::IPCAttachedPropertyFactory, QML_HAS_ATTACHED_PROPERTIES)
