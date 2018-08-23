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

#pragma once

#if defined(FaceliftIPCLibDBus_LIBRARY)
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_IMPORT
#endif

#include <memory>

#include <QDebug>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusVirtualObject>
#include <QDBusAbstractInterface>
#include <QDBusInterface>
#include <QDBusServiceWatcher>
#include <QDBusContext>

#include "FaceliftModel.h"
#include "FaceliftUtils.h"
#include "FaceliftProperty.h"

#include "ipc-common/ipc-common.h"

#include "ipc-dbus-serialization.h"

namespace facelift {

namespace ipc {
class ObjectRegistry;
}

namespace dbus {

using namespace facelift;


class FaceliftIPCLibDBus_EXPORT DBusIPCMessage
{

public:
    DBusIPCMessage() :
        DBusIPCMessage("dummy/dummy", "dummy.dummy", "gg")
    {
    }

    DBusIPCMessage(const QDBusMessage &msg)
    {
        m_message = msg;
    }

    DBusIPCMessage(const QString &service, const QString &path, const QString &interface, const QString &method)
    {
        m_message = QDBusMessage::createMethodCall(service, path, interface, method);
    }

    DBusIPCMessage(const QString &path, const QString &interface, const QString &signal)
    {
        m_message = QDBusMessage::createSignal(path, interface, signal);
    }

    DBusIPCMessage call(const QDBusConnection &connection)
    {
        qDebug() << "Sending IPC message : " << toString();
        auto replyDbusMessage = connection.call(m_message);
        DBusIPCMessage reply(replyDbusMessage);
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

    DBusIPCMessage createReply()
    {
        return DBusIPCMessage(m_message.createReply());
    }

    DBusIPCMessage createErrorReply(const QString &msg, const QString &member)
    {
        return DBusIPCMessage(m_message.createErrorReply(msg, member));
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

    static void write(DBusIPCMessage &msg, const Type &v)
    {
        msg.writeSimple(v);
    }

    static void read(DBusIPCMessage &msg, Type &v)
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


struct FaceliftIPCLibDBus_EXPORT AppendDBUSSignatureFunction
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

    static void write(DBusIPCMessage &msg, const float &v)
    {
        msg.writeSimple((double)v);
    }

    static void read(DBusIPCMessage &msg, float &v)
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

    static void write(DBusIPCMessage &msg, const bool &v)
    {
        msg.writeSimple(v);
    }

    static void read(DBusIPCMessage &msg, bool &v)
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

    static void write(DBusIPCMessage &msg, const QString &v)
    {
        msg.writeSimple(v);
    }

    static void read(DBusIPCMessage &msg, QString &v)
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

    static void write(DBusIPCMessage &msg, const Type &param)
    {
        for_each_in_tuple_const(param.asTuple(), StreamWriteFunction<DBusIPCMessage>(msg));
        param.id();
        msg << param.id();
    }

    static void read(DBusIPCMessage &msg, Type &param)
    {
        typename Type::FieldTupleTypes tuple;
        for_each_in_tuple(tuple, StreamReadFunction<DBusIPCMessage>(msg));
        param.setValue(tuple);
        ModelElementID id;
        msg.readNextParameter(id);
        param.setId(id);
    }

};


template<typename Type>
struct IPCTypeHandler<Type *, typename std::enable_if<std::is_base_of<InterfaceBase, Type>::value>::type>
{
    static void writeDBUSSignature(QTextStream &s)
    {
        Q_UNUSED(s);
        Q_ASSERT(false);
    }

    static void write(DBusIPCMessage &msg, const Type *param)
    {
        Q_UNUSED(msg);
        Q_UNUSED(param);
        Q_ASSERT(false);
    }

    static void read(DBusIPCMessage &msg, Type *param)
    {
        Q_UNUSED(msg);
        Q_UNUSED(param);
        Q_ASSERT(false);
    }
};


template<typename Type>
struct IPCTypeHandler<Type, typename std::enable_if<std::is_enum<Type>::value>::type>
{
    static void writeDBUSSignature(QTextStream &s)
    {
        s << "i";
    }

    static void write(DBusIPCMessage &msg, const Type &param)
    {
        msg.writeSimple(static_cast<int>(param));
    }

    static void read(DBusIPCMessage &msg, Type &param)
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

    static void write(DBusIPCMessage &msg, const QList<ElementType> &list)
    {
        int count = list.size();
        msg.writeSimple(count);
        for (const auto &e : list) {
            IPCTypeHandler<ElementType>::write(msg, e);
        }
    }

    static void read(DBusIPCMessage &msg, QList<ElementType> &list)
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


template<typename ElementType>
struct IPCTypeHandler<QMap<QString, ElementType> >
{
    static void writeDBUSSignature(QTextStream &s)
    {
        s << "a{sv}";     // TODO: is it so?
        IPCTypeHandler<ElementType>::writeDBUSSignature(s);
    }

    static void write(DBusIPCMessage &msg, const QMap<QString, ElementType> &map)
    {
        int count = map.size();
        msg.writeSimple(count);
        for (auto i = map.constBegin(); i != map.constEnd(); ++i) {
            IPCTypeHandler<QString>::write(msg, i.key());
            IPCTypeHandler<ElementType>::write(msg, i.value());
        }
    }

    static void read(DBusIPCMessage &msg, QMap<QString, ElementType> &map)
    {
        map.clear();
        int count;
        msg.readNextParameter(count);
        for (int i = 0; i < count; i++) {
            QString key;
            ElementType value;
            IPCTypeHandler<QString>::read(msg, key);
            IPCTypeHandler<ElementType>::read(msg, value);
            map.insert(key, value);
        }
    }
};


template<typename Type>
DBusIPCMessage &operator<<(DBusIPCMessage &msg, const Type &v)
{
    IPCTypeHandler<Type>::write(msg, v);
    return msg;
}


template<typename Type>
DBusIPCMessage &operator>>(DBusIPCMessage &msg, Type &v)
{
    IPCTypeHandler<Type>::read(msg, v);
    return msg;
}


template<typename Type>
DBusIPCMessage &operator>>(DBusIPCMessage &msg, Property<Type> &property)
{
    Type v;
    IPCTypeHandler<Type>::read(msg, v);
    property.setValue(v);
    return msg;
}


class DBusObjectRegistry;

class FaceliftIPCLibDBus_EXPORT DBusManager
{

public:
    DBusManager();

    static DBusManager &instance();

    bool isDBusConnected() const
    {
        return m_dbusConnected;
    }

    bool registerServiceName(const QString &serviceName)
    {
        qDebug() << "Registering serviceName " << serviceName;
        auto success = m_busConnection.registerService(serviceName);
        return success;
    }

    QDBusConnection &connection()
    {
        return m_busConnection;
    }

    QString serviceName() const
    {
        return m_busConnection.baseService();
    }

    facelift::ipc::ObjectRegistry &objectRegistry();

private:
    QDBusConnection m_busConnection;
    DBusObjectRegistry *m_objectRegistry = nullptr;
    bool m_dbusConnected;
};


class FaceliftIPCLibDBus_EXPORT DBusIPCServiceAdapterBase : public IPCServiceAdapterBase
{
    Q_OBJECT

public:
    static constexpr const char *GET_PROPERTIES_MESSAGE_NAME = "GetAllProperties";
    static constexpr const char *PROPERTIES_CHANGED_SIGNAL_NAME = "PropertiesChanged";
    static constexpr const char *SIGNAL_TRIGGERED_SIGNAL_NAME = "SignalTriggered";
    static constexpr const char *SET_PROPERTY_MESSAGE_NAME = "SetProperty";
    static constexpr const char *INTROSPECTABLE_INTERFACE_NAME = "org.freedesktop.DBus.Introspectable";
    static constexpr const char *PROPERTIES_INTERFACE_NAME = "org.freedesktop.DBus.Properties";

    class DBusVirtualObject : public QDBusVirtualObject
    {

    public:
        DBusVirtualObject(DBusIPCServiceAdapterBase &adapter) : QDBusVirtualObject(nullptr), m_adapter(adapter)
        {
        }

        QString introspect(const QString &path) const
        {
            return m_adapter.introspect(path);
        }

        bool handleMessage(const QDBusMessage &message, const QDBusConnection &connection)
        {
            return m_adapter.handleMessage(message, connection);
        }

    private:
        DBusIPCServiceAdapterBase &m_adapter;
    };

    DBusIPCServiceAdapterBase(QObject *parent = nullptr) : IPCServiceAdapterBase(parent), m_dbusVirtualObject(*this)
    {
    }

    ~DBusIPCServiceAdapterBase();

    virtual QString introspect(const QString &path) const = 0;

    bool handleMessage(const QDBusMessage &dbusMsg, const QDBusConnection &connection);

    void onPropertyValueChanged()
    {
        DBusIPCMessage msg(objectPath(), interfaceName(), PROPERTIES_CHANGED_SIGNAL_NAME);
        serializePropertyValues(msg);
        msg.send(dbusManager().connection());
    }

    template<typename ... Args>
    void sendSignal(const QString& signalName, const Args & ... args)
    {
        DBusIPCMessage msg(objectPath(), interfaceName(), SIGNAL_TRIGGERED_SIGNAL_NAME);
        msg << signalName;
        auto argTuple = std::make_tuple(args ...);
        for_each_in_tuple(argTuple, StreamWriteFunction<DBusIPCMessage>(msg));
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

    virtual IPCHandlingResult handleMethodCallMessage(DBusIPCMessage &requestMessage, DBusIPCMessage &replyMessage) = 0;

    void serializePropertyValues(DBusIPCMessage &msg)
    {
        msg << m_service->ready();
        serializeSpecificPropertyValues(msg);
    }

    virtual void serializeSpecificPropertyValues(DBusIPCMessage &msg) = 0;

    void init(InterfaceBase *service);

    DBusManager &dbusManager()
    {
        return DBusManager::instance();
    }

protected:
    DBusVirtualObject m_dbusVirtualObject;

    QString m_introspectionData;
    QString m_serviceName;

    InterfaceBase *m_service = nullptr;

    bool m_alreadyInitialized = false;
};



template<typename ServiceType>
class DBusIPCServiceAdapter : public DBusIPCServiceAdapterBase
{
public:
    typedef ServiceType TheServiceType;

    DBusIPCServiceAdapter(QObject *parent) : DBusIPCServiceAdapterBase(parent)
    {
        setInterfaceName(ServiceType::FULLY_QUALIFIED_INTERFACE_NAME);
    }

    ServiceType *service() const override
    {
        return m_service;
    }

    void setService(InterfaceBase *service) override
    {
        m_service = toProvider<ServiceType>(service);
    }

    virtual void appendDBUSIntrospectionData(QTextStream &s) const = 0;

    void init() override final
    {
        DBusIPCServiceAdapterBase::init(m_service);
    }

    QString introspect(const QString &path) const override
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

    void connectModel(const QString& name, facelift::ModelBase &model)
    {
        connect(&model, static_cast<void (facelift::ModelBase::*)(int, int)>
                (&facelift::ModelBase::dataChanged), this, [this, name] (int first, int last) {
            this->sendSignal(name + "DataChanged", first, last);
        });
        connect(&model, &facelift::ModelBase::beginRemoveElements, this, [this, name] (int first, int last) {
            this->sendSignal(name + "BeginRemove", first, last);
        });
        connect(&model, &facelift::ModelBase::endRemoveElements, this, [this, name] () {
            this->sendSignal(name + "EndRemove");
        });
        connect(&model, &facelift::ModelBase::beginInsertElements, this, [this, name] (int first, int last) {
            this->sendSignal(name + "BeginInsert", first, last);
        });
        connect(&model, &facelift::ModelBase::endInsertElements, this, [this, name] () {
            this->sendSignal(name + "EndInsert");
        });
        connect(&model, &facelift::ModelBase::beginResetModel, this, [this, name] () {
            this->sendSignal(name + "BeginReset");
        });
        connect(&model, &facelift::ModelBase::endResetModel, this, [this, name] () {
            this->sendSignal(name + "EndReset");
        });
    }

    template<typename ElementType>
    void handleModelRequest(facelift::Model<ElementType>& model,
                            facelift::dbus::DBusIPCMessage &requestMessage,
                            facelift::dbus::DBusIPCMessage &replyMessage)
    {
        int first, last;
        requestMessage >> first >> last;
        QList<ElementType> list;

        // Make sure we do not request items which are out of range
        first = qMin(first, model.size() - 1);
        last = qMin(last, model.size() - 1);

        for (int i = first; i <= last; ++i)
            list.append(model.elementAt(i));

        replyMessage << list;
    }

protected:
    QPointer<ServiceType> m_service;

};

class FaceliftIPCLibDBus_EXPORT IPCRequestHandler
{

public:
    virtual ~IPCRequestHandler()
    {
    }

    virtual void deserializePropertyValues(DBusIPCMessage &msg) = 0;
    virtual void deserializeSignal(DBusIPCMessage &msg) = 0;
    virtual void setServiceRegistered(bool isRegistered) = 0;

};

class FaceliftIPCLibDBus_EXPORT DBusIPCProxyBinder : public IPCProxyBinderBase
{
    Q_OBJECT

public:
    Q_PROPERTY(QString serviceName READ serviceName WRITE setServiceName)
    Q_PROPERTY(QString interfaceName READ interfaceName WRITE setInterfaceName)

    DBusIPCProxyBinder(QObject *parent = nullptr) : IPCProxyBinderBase(parent)
    {
        m_busWatcher.setWatchMode(QDBusServiceWatcher::WatchForRegistration);
    }

    const QString &serviceName() const
    {
        return m_serviceName;
    }

    void setServiceName(const QString &name)
    {
        m_serviceName = name;
        m_explicitServiceName = true;
        checkInit();
    }

    const QString &interfaceName() const
    {
        return m_interfaceName;
    }

    void setInterfaceName(const QString &name)
    {
        m_interfaceName = name;
        checkInit();
    }

    Q_SLOT
    void onPropertiesChanged(const QDBusMessage &dbusMessage)
    {
        DBusIPCMessage msg(dbusMessage);
        m_serviceObject->deserializePropertyValues(msg);
    }

    Q_SLOT
    void onSignalTriggered(const QDBusMessage &dbusMessage)
    {
        if (!inProcess()) {   // TODO: onSignalTriggered should not be called in-process
            DBusIPCMessage msg(dbusMessage);
            m_serviceObject->deserializeSignal(msg);
        }
    }

    void bindToIPC();

    void onServiceAvailable()
    {
        requestPropertyValues();
    }

    void requestPropertyValues()
    {
        DBusIPCMessage msg(serviceName(), objectPath(), interfaceName(), DBusIPCServiceAdapterBase::GET_PROPERTIES_MESSAGE_NAME);
        auto replyMessage = msg.call(connection());
        if (replyMessage.isReplyMessage()) {
            m_serviceObject->deserializePropertyValues(replyMessage);
            m_serviceObject->setServiceRegistered(true);
        } else {
            qDebug() << "Service not yet available : " << objectPath();
        }
    }

    template<typename PropertyType>
    DBusIPCMessage sendSetterCall(const QString& methodName, const PropertyType &value)
    {
        DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
        msg << value;
        auto replyMessage = msg.call(connection());
        if (replyMessage.isErrorMessage()) {
            qCritical(
                "Error message received when calling method '%s' on service at path '%s'. This likely indicates that the server you are trying to access is not available yet",
                qPrintable(methodName), qPrintable(objectPath()));
        }
        return replyMessage;
    }

    template<typename ... Args>
    DBusIPCMessage sendMethodCall(const QString& methodName, const Args & ... args)
    {
        DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
        auto argTuple = std::make_tuple(args ...);
        for_each_in_tuple(argTuple, StreamWriteFunction<DBusIPCMessage>(msg));
        auto replyMessage = msg.call(connection());
        if (replyMessage.isErrorMessage()) {
            qCritical(
                "Error message received when calling method '%s' on service at path '%s'. This likely indicates that the server you are trying to access is not available yet",
                qPrintable(methodName), qPrintable(objectPath()));
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
        checkInit();
    }

private:
    QString m_serviceName;
    QString m_interfaceName;

    IPCRequestHandler *m_serviceObject = nullptr;

    facelift::ipc::ObjectRegistry *m_objectRegistry = nullptr;

    QDBusServiceWatcher m_busWatcher;
    bool m_explicitServiceName = false;

};



template<typename AdapterType, typename IPCAdapterType>
class DBusIPCProxy : public IPCProxyBase<AdapterType, IPCAdapterType>, protected IPCRequestHandler
{

public:
    DBusIPCProxy(QObject *parent = nullptr) :
        IPCProxyBase<AdapterType, IPCAdapterType>(parent)
    {
        m_ipcBinder.setInterfaceName(AdapterType::FULLY_QUALIFIED_INTERFACE_NAME);
        m_ipcBinder.setHandler(this);

        this->initBinder(m_ipcBinder);
        this->setImplementationID("DBus IPC Proxy");
    }

    virtual void deserializeSpecificPropertyValues(DBusIPCMessage &msg) = 0;

    void deserializePropertyValues(DBusIPCMessage &msg) override
    {
        auto r = this->ready();
        msg >> r;
        this->setReady(r);
        deserializeSpecificPropertyValues(msg);
    }

    void setServiceRegistered(bool isRegistered) override
    {
        bool oldReady = this->ready();
        m_serviceRegistered = isRegistered;
        if (this->ready() != oldReady) {
            this->readyChanged();
        }
    }

    template<typename PropertyType>
    void sendSetterCall(const char *methodName, const PropertyType &value)
    {
        DBusIPCMessage msg = m_ipcBinder.sendSetterCall(methodName, value);
        if (msg.isReplyMessage()) {
            deserializeSpecificPropertyValues(msg);
        }
    }

    template<typename ... Args>
    void sendMethodCall(const QString& methodName, const Args & ... args)
    {
        DBusIPCMessage msg = m_ipcBinder.sendMethodCall(methodName, args ...);
        if (msg.isReplyMessage()) {
            deserializeSpecificPropertyValues(msg);
        }
    }

    template<typename ReturnType, typename ... Args>
    void sendMethodCallWithReturn(const QString& methodName, ReturnType &returnValue, const Args & ... args)
    {
        DBusIPCMessage msg = m_ipcBinder.sendMethodCall(methodName, args ...);
        if (msg.isReplyMessage()) {
            msg >> returnValue;
            deserializeSpecificPropertyValues(msg);
        }
    }

    template<typename ReturnType, typename ... Args>
    void sendMethodCallWithReturnNoSync(const QString& methodName, ReturnType &returnValue, const Args & ... args)
    {
        DBusIPCMessage msg = m_ipcBinder.sendMethodCall(methodName, args ...);
        if (msg.isReplyMessage()) {
            msg >> returnValue;
        }
    }

    template<typename ElementType>
    void handleModelSignal(facelift::Model<ElementType>& model,
                           facelift::MostRecentlyUsedCache<int, ElementType>& cache, const QString& modelName,
                           const QString& signalName, facelift::dbus::DBusIPCMessage &msg)
    {
        if (signalName == modelName + "DataChanged") {
            int first, last;
            msg >> first >> last;
            for (int i = first; i <= last; ++i) {
                if (cache.exists(i))
                    cache.remove(i);
            }
            emit model.dataChanged(first, last);
        } else if (signalName == modelName + "BeginInsert") {
            cache.clear();
            int first, last;
            msg >> first >> last;
            emit model.beginInsertElements(first, last);
        } else if (signalName == modelName + "EndInsert") {
            emit model.endInsertElements();
        } else if (signalName == modelName + "BeginRemove") {
            cache.clear();
            int first, last;
            msg >> first >> last;
            emit model.beginRemoveElements(first, last);
        } else if (signalName == modelName + "EndRemove") {
            emit model.endRemoveElements();
        } else if (signalName == modelName + "BeginReset") {
            cache.clear();
            emit model.beginResetModel();
        } else if (signalName == modelName + "EndReset") {
            cache.clear();
            emit model.endResetModel();
        }
    }

    template<typename ElementType>
    ElementType modelData(facelift::Model<ElementType>& model,
                          facelift::MostRecentlyUsedCache<int, ElementType>& cache, const QString& modelName, int row)
    {
        ElementType retval;
        if (cache.exists(row)) {
            retval = cache.get(row);
        } else {
            static const int prefetch = 12;    // fetch 25 items around requested one
            QList<ElementType> list;
            int first = row > prefetch ? row - prefetch : 0;
            int last = row < model.size() - prefetch ? row + prefetch : model.size() - 1;

            while (cache.exists(first) && first < last)
                ++first;
            while (cache.exists(last) && last > first)
                --last;

            this->sendMethodCallWithReturnNoSync(modelName + "Data", list, first, last);
            Q_ASSERT(list.size() == (last - first + 1));

            for (int i = first; i <= last; ++i)
                cache.insert(i, list.at(i - first));

            retval = list.at(row - first);
        }
        return retval;
    }

    DBusIPCProxyBinder *ipc()
    {
        return &m_ipcBinder;
    }

    void connectToServer()
    {
        m_ipcBinder.connectToServer();
    }

private:
    DBusIPCProxyBinder m_ipcBinder;
    bool m_serviceRegistered = false;

};


class FaceliftIPCLibDBus_EXPORT DBusIPCAdapterFactoryManager
{
public:
    typedef DBusIPCServiceAdapterBase * (*IPCAdapterFactory)(InterfaceBase *);

    static DBusIPCAdapterFactoryManager &instance();

    template<typename AdapterType>
    static DBusIPCServiceAdapterBase *createInstance(InterfaceBase *i)
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
            i.m_factories.insert(typeID, &DBusIPCAdapterFactoryManager::createInstance<AdapterType>);
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


class FaceliftIPCLibDBus_EXPORT DBusIPCAttachedPropertyFactory : public IPCAttachedPropertyFactoryBase
{
public:
    static DBusIPCServiceAdapterBase *qmlAttachedProperties(QObject *object);

};

}


class FaceliftIPCLibDBus_EXPORT InterfacePropertyHandlerBase
{
public:
    QString generateObjectPath(const QString &parentPath)
    {
        QString path = parentPath + "/dynamic";
        path += QString::number(s_nextInstanceID++);
        return path;
    }

    static int s_nextInstanceID;
};


template<typename InterfaceType, typename InterfaceAdapterType>
class InterfacePropertyIPCAdapterHandler : public InterfacePropertyHandlerBase
{

public:
    void update(IPCServiceAdapterBase *parent, InterfaceType *service)
    {
        if (m_service != service) {
            m_service = service;
            if (m_service) {
                m_serviceAdapter = new InterfaceAdapterType(service); // This object will be deleted together with the service itself
                m_serviceAdapter->setObjectPath(generateObjectPath(parent->objectPath()));
                m_serviceAdapter->setService(service);
                m_serviceAdapter->init();
            }
        }
    }

    QString objectPath() const
    {
        if (m_serviceAdapter) {
            return m_serviceAdapter->objectPath();
        } else {
            return "";
        }
    }


    QPointer<InterfaceType> m_service;
    QPointer<InterfaceAdapterType> m_serviceAdapter;
};


template<typename ProxyType>
class InterfacePropertyIPCProxyHandler : public InterfacePropertyHandlerBase
{

public:
    void update(const QString &objectPath)
    {
        if (m_proxy && (m_proxy->ipc()->objectPath() != objectPath)) {
            m_proxy = nullptr;
        }
        if (!m_proxy) {
            m_proxy = std::move(std::unique_ptr<ProxyType>(new ProxyType()));
            m_proxy->ipc()->setObjectPath(objectPath);
            m_proxy->connectToServer();
        }
    }

    ProxyType *getValue() const
    {
        return m_proxy.get();
    }

private:
    std::unique_ptr<ProxyType> m_proxy;
};

}

QML_DECLARE_TYPEINFO(facelift::dbus::DBusIPCAttachedPropertyFactory, QML_HAS_ATTACHED_PROPERTIES)
