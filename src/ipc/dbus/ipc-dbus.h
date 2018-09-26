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
#include <QDataStream>
#include <QByteArray>
#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>


#include "FaceliftModel.h"
#include "FaceliftUtils.h"
#include "FaceliftProperty.h"

#include "ipc-common/ipc-common.h"

namespace facelift {

namespace ipc { namespace dbus {
class ObjectRegistry;
} }

namespace dbus {

using namespace facelift;

class OutputPayLoad;
class InputPayLoad;

class FaceliftIPCLibDBus_EXPORT DBusIPCMessage
{

public:
    DBusIPCMessage() :
        DBusIPCMessage("dummy/dummy", "dummy.dummy", "gg")
    {
    }

    DBusIPCMessage(const DBusIPCMessage &other) : m_message(other.m_message)
    {
    }

    DBusIPCMessage & operator=(const DBusIPCMessage &other) {
        m_message = other.m_message;
        return *this;
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

    DBusIPCMessage call(const QDBusConnection &connection);

    void asyncCall(const QDBusConnection &connection, const QObject* context, std::function<void(DBusIPCMessage& message)> callback);

    void send(const QDBusConnection &connection);

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

    OutputPayLoad& outputPayLoad();

    InputPayLoad& inputPayLoad();

private:

    QDBusMessage m_message;
    std::unique_ptr<OutputPayLoad> m_outputPayload;
    std::unique_ptr<InputPayLoad> m_inputPayload;
};


class OutputPayLoad {

public:
    OutputPayLoad() : m_dataStream(&m_payloadArray, QIODevice::WriteOnly)
    {
    }

    template<typename Type>
    void writeSimple(const Type &v)
    {
//        qDebug() << "Writing to message : " << v;
        m_dataStream << v;
    }

    const QByteArray& getContent() const {
        return m_payloadArray;
    }

private:
    QByteArray m_payloadArray;
    QDataStream m_dataStream;
};

class InputPayLoad {

public:
    InputPayLoad(const QByteArray& payloadArray) : m_payloadArray(payloadArray), m_dataStream(m_payloadArray)
    {
    }

    template<typename Type>
    void readNextParameter(Type &v)
    {
        m_dataStream >> v;
//        qDebug() << "Read from message : " << v;
    }

private:
    QByteArray m_payloadArray;
    QDataStream m_dataStream;
};


inline OutputPayLoad& DBusIPCMessage::outputPayLoad() {
    if (m_outputPayload == nullptr) {
        m_outputPayload = std::make_unique<OutputPayLoad>();
    }
    return *m_outputPayload;
}

inline InputPayLoad& DBusIPCMessage::inputPayLoad() {
    if (m_inputPayload == nullptr) {
        auto byteArray = m_message.arguments()[0].value<QByteArray>();
        m_inputPayload = std::make_unique<InputPayLoad>(byteArray);
    }
    return *m_inputPayload;
}


class DBusIPCServiceAdapterBase;



template<typename Type, typename Enable = void>
struct IPCTypeHandler
{
    static void writeDBUSSignature(QTextStream &s)
    {
        s << "i";
    }

    static void write(OutputPayLoad &msg, const Type &v)
    {
        msg.writeSimple(v);
    }

    static void read(InputPayLoad &msg, Type &v)
    {
        msg.readNextParameter(v);
    }

};


template<>
struct IPCTypeHandler<float>
{
    static void writeDBUSSignature(QTextStream &s)
    {
        s << "d";
    }

    static void write(OutputPayLoad &msg, const float &v)
    {
        msg.writeSimple((double)v);
    }

    static void read(InputPayLoad &msg, float &v)
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

    static void write(OutputPayLoad &msg, const bool &v)
    {
        msg.writeSimple(v);
    }

    static void read(InputPayLoad &msg, bool &v)
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

    static void write(OutputPayLoad &msg, const QString &v)
    {
        msg.writeSimple(v);
    }

    static void read(InputPayLoad &msg, QString &v)
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

    static void write(OutputPayLoad &msg, const Type &param)
    {
        for_each_in_tuple_const(param.asTuple(), StreamWriteFunction<OutputPayLoad>(msg));
        param.id();
        msg << param.id();
    }

    static void read(InputPayLoad &msg, Type &param)
    {
        typename Type::FieldTupleTypes tuple;
        for_each_in_tuple(tuple, StreamReadFunction<InputPayLoad>(msg));
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

    static void write(OutputPayLoad &msg, const Type &param)
    {
        msg.writeSimple(static_cast<int>(param));
    }

    static void read(InputPayLoad &msg, Type &param)
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

    static void write(OutputPayLoad &msg, const QList<ElementType> &list)
    {
        int count = list.size();
        msg.writeSimple(count);
        for (const auto &e : list) {
            IPCTypeHandler<ElementType>::write(msg, e);
        }
    }

    static void read(InputPayLoad &msg, QList<ElementType> &list)
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

    static void write(OutputPayLoad &msg, const QMap<QString, ElementType> &map)
    {
        int count = map.size();
        msg.writeSimple(count);
        for (auto i = map.constBegin(); i != map.constEnd(); ++i) {
            IPCTypeHandler<QString>::write(msg, i.key());
            IPCTypeHandler<ElementType>::write(msg, i.value());
        }
    }

    static void read(InputPayLoad &msg, QMap<QString, ElementType> &map)
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



template<typename Type>
OutputPayLoad &operator<<(OutputPayLoad &msg, const Type &v)
{
    IPCTypeHandler<Type>::write(msg, v);
    return msg;
}


template<typename Type>
InputPayLoad &operator>>(InputPayLoad &msg, Type &v)
{
    IPCTypeHandler<Type>::read(msg, v);
    return msg;
}


template<typename Type>
InputPayLoad &operator>>(InputPayLoad &msg, Property<Type> &property)
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

    facelift::ipc::dbus::ObjectRegistry &objectRegistry();

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

    struct SerializeParameterFunction
    {
        SerializeParameterFunction(OutputPayLoad &msg, DBusIPCServiceAdapterBase &parent) :
            m_msg(msg),
            m_parent(parent)
        {
        }

        OutputPayLoad &m_msg;
        DBusIPCServiceAdapterBase &m_parent;

        template<typename Type>
        void operator()(const Type &v)
        {
            IPCTypeHandler<typename IPCTypeRegisterHandler<Type>::SerializedType>::write(m_msg,
                    IPCTypeRegisterHandler<Type>::convertToSerializedType(v, m_parent));
        }
    };


    template<typename Type>
    void serializeValue(DBusIPCMessage &msg, const Type &v)
    {
        typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
        IPCTypeHandler<SerializedType>::write(msg.outputPayLoad(), IPCTypeRegisterHandler<Type>::convertToSerializedType(v, *this));
    }

    template<typename Type>
    void deserializeValue(DBusIPCMessage &msg, Type &v)
    {
        typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
        SerializedType serializedValue;
        IPCTypeHandler<Type>::read(msg.inputPayLoad(), serializedValue);
        IPCTypeRegisterHandler<Type>::convertToDeserializedType(v, serializedValue, *this);
    }

    template<typename ... Args>
    void sendSignal(const QString &signalName, const Args & ... args)
    {
        DBusIPCMessage msg(objectPath(), interfaceName(), SIGNAL_TRIGGERED_SIGNAL_NAME);
        serializeValue(msg, signalName);
        auto argTuple = std::make_tuple(args ...);
        for_each_in_tuple(argTuple, SerializeParameterFunction(msg.outputPayLoad(), *this));
        msg.send(dbusManager().connection());
    }

    template<typename ReturnType>
    void sendAsyncCallAnswer(DBusIPCMessage& replyMessage, const ReturnType returnValue) {
        serializeValue(replyMessage, returnValue);
        replyMessage.send(dbusManager().connection());
    }

    void sendAsyncCallAnswer(DBusIPCMessage& replyMessage) {
        replyMessage.send(dbusManager().connection());
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

    virtual void serializePropertyValues(DBusIPCMessage &msg)
    {
        serializeValue(msg, m_service->ready());
    }

    void doInit(InterfaceBase *service);

    DBusManager &dbusManager()
    {
        return DBusManager::instance();
    }

    InterfaceBase *service() const
    {
        return m_service;
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
    typedef const char *MemberIDType;

    DBusIPCServiceAdapter(QObject *parent) : DBusIPCServiceAdapterBase(parent)
    {
        setInterfaceName(ServiceType::FULLY_QUALIFIED_INTERFACE_NAME);
    }

    template<typename T>
    MemberIDType memberID(T member, const char *memberName) const
    {
        // DBus member IDs are strings
        Q_UNUSED(member);
        return memberName;
    }

    ServiceType *service() const override
    {
        return m_service;
    }

    void setService(QObject *service) override
    {
        m_service = bindToProvider<ServiceType>(service);
    }

    virtual void appendDBUSIntrospectionData(QTextStream &s) const = 0;

    void init() override final
    {
        DBusIPCServiceAdapterBase::doInit(m_service);
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

protected:
    QPointer<ServiceType> m_service;
};

class FaceliftIPCLibDBus_EXPORT DBusRequestHandler
{

public:
    virtual ~DBusRequestHandler()
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

    DBusIPCProxyBinder(InterfaceBase &owner, QObject *parent = nullptr) : IPCProxyBinderBase(owner, parent)
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

        auto replyHandler = [this] (DBusIPCMessage& replyMessage) {
            if (replyMessage.isReplyMessage()) {
                m_serviceObject->deserializePropertyValues(replyMessage);
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

    template<typename Type>
    void serializeValue(DBusIPCMessage &msg, const Type &v)
    {
        typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
        IPCTypeHandler<SerializedType>::write(msg.outputPayLoad(), IPCTypeRegisterHandler<Type>::convertToSerializedType(v, *this));
    }

    template<typename Type>
    void deserializeValue(DBusIPCMessage &msg, Type &v)
    {
        typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
        SerializedType serializedValue;
        IPCTypeHandler<SerializedType>::read(msg.inputPayLoad(), serializedValue);
        IPCTypeRegisterHandler<Type>::convertToDeserializedType(v, serializedValue, *this);
    }

    void onServerNotAvailableError(const char *methodName) const {
        qCritical(
            "Error message received when calling method '%s' on service at path '%s'. "
            "This likely indicates that the server you are trying to access is not available yet",
            qPrintable(methodName), qPrintable(objectPath()));
    }

    template<typename PropertyType>
    void sendSetterCall(const char* methodName, const PropertyType &value)
    {
        DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
        serializeValue(msg, value);
        if (isSynchronous()) {
            auto replyMessage = msg.call(connection());
            if (replyMessage.isErrorMessage()) {
                onServerNotAvailableError(methodName);
            }
        } else {
            msg.asyncCall(connection(), this, [this, methodName](const DBusIPCMessage& replyMessage){
                if (replyMessage.isErrorMessage()) {
                    onServerNotAvailableError(methodName);
                }
            });
        }
    }

    struct SerializeParameterFunction
    {
        SerializeParameterFunction(DBusIPCMessage &msg, const DBusIPCProxyBinder &parent) :
            m_msg(msg),
            m_parent(parent)
        {
        }

        DBusIPCMessage &m_msg;
        const DBusIPCProxyBinder &m_parent;

        template<typename Type>
        void operator()(const Type &v)
        {
            IPCTypeHandler<typename IPCTypeRegisterHandler<Type>::SerializedType>::write(m_msg.outputPayLoad(),
                    IPCTypeRegisterHandler<Type>::convertToSerializedType(v, m_parent));
        }
    };

    template<typename ... Args>
    DBusIPCMessage sendMethodCall(const char *methodName, const Args & ... args) const
    {
        DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
        auto argTuple = std::make_tuple(args ...);
        for_each_in_tuple(argTuple, SerializeParameterFunction(msg, *this));
        auto replyMessage = msg.call(connection());
        if (replyMessage.isErrorMessage()) {
            onServerNotAvailableError(methodName);
        }
        return replyMessage;
    }

    template<typename ReturnType, typename ... Args>
    void sendAsyncMethodCall(const QString &methodName, facelift::AsyncAnswer<ReturnType> answer, const Args & ... args)
    {
        DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
        auto argTuple = std::make_tuple(args ...);
        for_each_in_tuple(argTuple, SerializeParameterFunction(msg, *this));
        msg.asyncCall(connection(), this, [this, answer](DBusIPCMessage& msg) {
            ReturnType returnValue;
            deserializeValue(msg, returnValue);
            answer(returnValue);
        });
    }

    template<typename ... Args>
    void sendAsyncMethodCall(const QString &methodName, facelift::AsyncAnswer<void> answer, const Args & ... args)
    {
        DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
        auto argTuple = std::make_tuple(args ...);
        for_each_in_tuple(argTuple, SerializeParameterFunction(msg, *this));
        msg.asyncCall(connection(), this, [answer](DBusIPCMessage& msg) {
            Q_UNUSED(msg);
            answer();
        });
    }


    QDBusConnection &connection() const
    {
        return manager().connection();
    }

    DBusManager &manager() const
    {
        return DBusManager::instance();
    }

    void setHandler(DBusRequestHandler *handler)
    {
        m_serviceObject = handler;
        checkInit();
    }

private:
    QString m_serviceName;
    QString m_interfaceName;

    DBusRequestHandler *m_serviceObject = nullptr;

    facelift::ipc::dbus::ObjectRegistry *m_objectRegistry = nullptr;

    QDBusServiceWatcher m_busWatcher;
    bool m_explicitServiceName = false;

};



template<typename AdapterType, typename IPCAdapterType>
class DBusIPCProxy : public IPCProxyBase<AdapterType, IPCAdapterType>, protected DBusRequestHandler
{
    using IPCProxyBase<AdapterType, IPCAdapterType>::assignDefaultValue;

public:
    typedef const char *MemberIDType;

    DBusIPCProxy(QObject *parent = nullptr) :
        IPCProxyBase<AdapterType, IPCAdapterType>(parent), m_ipcBinder(*this)
    {
        m_ipcBinder.setInterfaceName(AdapterType::FULLY_QUALIFIED_INTERFACE_NAME);
        m_ipcBinder.setHandler(this);

        this->initBinder(m_ipcBinder);
        this->setImplementationID("DBus IPC Proxy");
    }

    template<typename T>
    MemberIDType memberID(T member, const char *memberName) const
    {
        // DBus member IDs are strings
        Q_UNUSED(member);
        return memberName;
    }

    template<typename Type>
    void serializeValue(DBusIPCMessage &msg, const Type &v)
    {
        typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
        IPCTypeHandler<SerializedType>::write(msg, IPCTypeRegisterHandler<Type>::convertToSerializedType(v, *this));
    }

    template<typename Type>
    void deserializeValue(DBusIPCMessage &msg, Type &v)
    {
        typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
        SerializedType serializedValue;
        IPCTypeHandler<SerializedType>::read(msg.inputPayLoad(), serializedValue);
        IPCTypeRegisterHandler<Type>::convertToDeserializedType(v, serializedValue, *this);
    }

    void deserializePropertyValues(DBusIPCMessage &msg) override
    {
        auto r = this->ready();
        deserializeValue(msg, r);
        this->setReady(r);
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
        m_ipcBinder.sendSetterCall(methodName, value);
    }

    template<typename ... Args>
    void sendMethodCall(const char *methodName, const Args & ... args) const
    {
        m_ipcBinder.sendMethodCall(methodName, args ...);
    }

    template<typename ReturnType, typename ... Args>
    void sendMethodCallWithReturn(const char *methodName, ReturnType &returnValue, const Args & ... args) const
    {
        DBusIPCMessage msg = m_ipcBinder.sendMethodCall(methodName, args ...);
        if (msg.isReplyMessage()) {
            const_cast<DBusIPCProxy *>(this)->deserializeValue(msg, returnValue);
        } else {
            assignDefaultValue(returnValue);
        }
    }

    template<typename ReturnType, typename ... Args>
    void sendAsyncMethodCall(const QString &methodName, facelift::AsyncAnswer<ReturnType> answer, const Args & ... args) const
    {
        const_cast<DBusIPCProxy *>(this)->m_ipcBinder.sendAsyncMethodCall(methodName, answer, args...);
    }

    DBusIPCProxyBinder *ipc()
    {
        return &m_ipcBinder;
    }

    template<typename InterfaceType>
    typename InterfaceType::IPCProxyType *getOrCreateSubProxy(const QString &objectPath)
    {
        return m_ipcBinder.getOrCreateSubProxy<InterfaceType>(objectPath);
    }

    void connectToServer()
    {
        m_ipcBinder.connectToServer();
    }

private:
    DBusIPCProxyBinder m_ipcBinder;
    bool m_serviceRegistered = false;

};

}

}
