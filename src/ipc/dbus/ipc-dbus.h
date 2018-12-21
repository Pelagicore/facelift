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
class ObjectRegistryAsync;
} }

namespace dbus {

using namespace facelift;

class OutputPayLoad;
class InputPayLoad;

struct DBusIPCCommon {
    static constexpr const char *GET_PROPERTIES_MESSAGE_NAME = "GetAllProperties";
    static constexpr const char *PROPERTIES_CHANGED_SIGNAL_NAME = "PropertiesChanged";
    static constexpr const char *SIGNAL_TRIGGERED_SIGNAL_NAME = "SignalTriggered";
    static constexpr const char *SET_PROPERTY_MESSAGE_NAME = "SetProperty";
    static constexpr const char *INTROSPECTABLE_INTERFACE_NAME = "org.freedesktop.DBus.Introspectable";
    static constexpr const char *PROPERTIES_INTERFACE_NAME = "org.freedesktop.DBus.Properties";
};

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

    DBusIPCMessage &operator=(const DBusIPCMessage &other)
    {
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

    void asyncCall(const QDBusConnection &connection, const QObject *context, std::function<void(DBusIPCMessage &message)> callback);

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

    OutputPayLoad &outputPayLoad();

    InputPayLoad &inputPayLoad();

private:
    QDBusMessage m_message;
    std::unique_ptr<OutputPayLoad> m_outputPayload;
    std::unique_ptr<InputPayLoad> m_inputPayload;
};


class FaceliftIPCLibDBus_EXPORT OutputPayLoad
{

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

    const QByteArray &getContent() const
    {
        return m_payloadArray;
    }

private:
    QByteArray m_payloadArray;
    QDataStream m_dataStream;
};

class FaceliftIPCLibDBus_EXPORT InputPayLoad
{

public:
    InputPayLoad(const QByteArray &payloadArray) : m_payloadArray(payloadArray), m_dataStream(m_payloadArray)
    {
    }

    ~InputPayLoad()
    {
        Q_ASSERT(m_dataStream.atEnd());
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


inline OutputPayLoad &DBusIPCMessage::outputPayLoad()
{
    if (m_outputPayload == nullptr) {
        m_outputPayload = std::make_unique<OutputPayLoad>();
    }
    return *m_outputPayload;
}

inline InputPayLoad &DBusIPCMessage::inputPayLoad()
{
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
    using Type = decltype(std::get<I>(t));
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
    using Type = decltype(std::get<I>(t));
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
    using Type = decltype(std::get<I>(t));
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

    DBusObjectRegistry &objectRegistry();

private:
    QDBusConnection m_busConnection;
    DBusObjectRegistry *m_objectRegistry = nullptr;
    bool m_dbusConnected;
};



class FaceliftIPCLibDBus_EXPORT DBusRequestHandler
{

public:
    virtual ~DBusRequestHandler()
    {
    }

    virtual void deserializePropertyValues(DBusIPCMessage &msg, bool isCompleteSnapshot) = 0;
    virtual void deserializeSignal(DBusIPCMessage &msg) = 0;
    virtual void setServiceRegistered(bool isRegistered) = 0;

};


}

}
