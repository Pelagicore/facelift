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

#include <QObject>
#include <QQmlListProperty>
#include <QDebug>
#include <QQuickItem>
#include <QQmlEngine>
#include <QJSValue>
#include <QTimer>
#include <QMap>
#include <memory>

#include <array>

#include "FaceliftUtils.h"


#define STRINGIFY_(x) # x
#define STRINGIFY(x) STRINGIFY_(x)

namespace facelift {

typedef int ModelElementID;

template<typename ElementType>
using Map = QMap<QString, ElementType>;


class StructureBase
{
    Q_GADGET

public:
    // Q_PROPERTIES defined here are not visible in subclasses, for some reason (Qt bug ?)

    static constexpr int ROLE_ID = 1000;
    static constexpr int ROLE_BASE = ROLE_ID + 1;

    ModelElementID id() const
    {
        return m_id;
    }

    StructureBase()
    {
        m_id = s_nextID++;
    }

    virtual ~StructureBase()
    {
    }

    void setId(ModelElementID id)
    {
        m_id = id;
    }

    virtual QByteArray serialize() const = 0;

    virtual void deserialize(const QByteArray &) = 0;

protected:
    ModelElementID m_id;

private:
    static ModelElementID s_nextID;

};


template<typename Type>
QString enumToString(const Type &v)
{
    static_assert(!std::is_enum<Type>::value, "Missing specialization of enumToString() template");
    return "";
}


struct BinarySeralizer
{
    BinarySeralizer(QByteArray &array) : stream(&array, QIODevice::WriteOnly)
    {
    }
    BinarySeralizer(const QByteArray &array) : stream(array)
    {
    }
    QDataStream stream;
};

struct TypeHandlerBase
{

    template<typename Type>
    static void write(BinarySeralizer &msg, const Type &v)
    {
        msg.stream << v;
    }

    template<typename Type>
    static void read(BinarySeralizer &msg, Type &v)
    {
        msg.stream >> v;
    }

    template<typename Type>
    static QString toString(const Type &v)
    {
        NOT_IMPLEMENTED();
        return v;
    }

    template<typename ReceiverType, typename Function>
    static void connectChangeSignals(const QVariant &variant, ReceiverType *receiver, Function function,
            QList<QMetaObject::Connection> &connections)
    {
        // nothing to connect for most of the types
        Q_UNUSED(variant);
        Q_UNUSED(receiver);
        Q_UNUSED(function);
        Q_UNUSED(connections);
    }

    template<typename Type>
    static const Type &toQMLCompatibleType(const Type &v)
    {
        return v;
    }

    template<typename Type, typename QmlType>
    static void assignFromQmlType(Type &field, const QmlType &qmlValue)
    {
        field = qmlValue;
    }

};

template<typename Type, typename Enable = void>
struct TypeHandler
{
    typedef Type QMLType;

    static QString toString(const Type &v)
    {
        Q_UNUSED(v);
        return "Unknown";
    }

};


template<typename ... FieldTypes>
class Structure : public StructureBase
{

public:
    virtual ~Structure()
    {
    }

    typedef std::tuple<FieldTypes ...> FieldTupleTypes;
    static constexpr size_t FieldCount = sizeof ... (FieldTypes);

    typedef std::array<const char *, FieldCount> FieldNames;

    const FieldTupleTypes &asTuple() const
    {
        return m_values;
    }

    FieldTupleTypes &asTuple()
    {
        return m_values;
    }

    QByteArray serialize() const override
    {
        QByteArray array;
        BinarySeralizer ds(array);
        ds << *this;
        return array;
    }

    void deserialize(const QByteArray &array) override
    {
        BinarySeralizer ds(array);
        ds >> *this;
    }

    void setValue(FieldTupleTypes value)
    {
        m_values = value;
    }

    void copyFrom(const Structure &other)
    {
        setValue(other.m_values);
        m_id = other.id();
    }

    bool operator==(const Structure &right) const
    {
        return (m_values == right.m_values);
    }

protected:
    template<std::size_t I = 0, typename ... Tp>
    inline typename std::enable_if<I == sizeof ... (Tp), void>::type
    toStringWithFields(const std::tuple<Tp ...> &t, const FieldNames &names, QTextStream &outStream) const
    {
        Q_UNUSED(t);
        Q_UNUSED(names);
        Q_UNUSED(outStream);
    }

    template<std::size_t I = 0, typename ... Tp>
    inline typename std::enable_if < I<sizeof ... (Tp), void>::type
    toStringWithFields(const std::tuple<Tp ...> &t, const FieldNames &names, QTextStream &outStream) const
    {
        typedef typename std::tuple_element<I, std::tuple<Tp ...> >::type TupleElementType;
        outStream << ", ";
        outStream << names[I] << "=" << TypeHandler<TupleElementType>::toString(std::get<I>(t));
        //        if (I != FieldCount) {        }
        toStringWithFields<I + 1, Tp ...>(t, names, outStream);
    }

    QString toStringWithFields(const QString &structName, const FieldNames &names) const
    {
        QString s;
        QTextStream outStream(&s);
        outStream << structName << " { id=" << id();
        toStringWithFields(m_values, names, outStream);
        outStream << "}";

        return s;
    }

    FieldTupleTypes m_values = {};

};


template<typename Type>
BinarySeralizer &operator<<(BinarySeralizer &msg, const Type &v)
{
    TypeHandler<Type>::write(msg, v);
    return msg;
}


template<typename Type>
BinarySeralizer &operator>>(BinarySeralizer &msg, Type &v)
{
    TypeHandler<Type>::read(msg, v);
    return msg;
}

template<typename Type>
struct TypeHandler<Type, typename std::enable_if<std::is_base_of<StructureBase, Type>::value>::type>
{
    typedef Type QMLType;

    static void write(BinarySeralizer &msg, const Type &param)
    {
        auto tupleCopy = param.asTuple();
        for_each_in_tuple(tupleCopy, StreamWriteFunction<BinarySeralizer>(msg));
        param.id();
        msg << param.id();
    }

    static void read(BinarySeralizer &msg, Type &param)
    {
        typename Type::FieldTupleTypes tuple;
        for_each_in_tuple(tuple, StreamReadFunction<BinarySeralizer>(msg));
        param.setValue(tuple);
        ModelElementID id;
        msg >> id;
        param.setId(id);
    }

    static Type fromVariant(const QVariant &variant)
    {
        Type v;
        typedef typename Type::QObjectWrapperType QObjectWrapperType;

        if (variant.canConvert<QObjectWrapperType *>()) {
            auto qobjectWrapper = variant.value<QObjectWrapperType *>();
            v = qobjectWrapper->gadget();               // TODO : check how to react on changes in the wrapper QObject
        } else if (variant.canConvert<Type>()) {
            v = variant.value<Type>();
        } else {
            qFatal("Bad argument");
        }

        return v;
    }

    template<typename ReceiverType, typename Function>
    static void connectChangeSignals(const QVariant &variant, ReceiverType *receiver, Function function,
            QList<QMetaObject::Connection> &connections)
    {
        typedef typename Type::QObjectWrapperType QObjectWrapperType;
        if (variant.canConvert<QObjectWrapperType *>()) {
            auto qobjectWrapper = variant.value<QObjectWrapperType *>();
            connections.append(QObject::connect(qobjectWrapper, &QObjectWrapperType::anyFieldChanged, receiver, function));
        }
    }

    static QString toString(const Type &v)
    {
        return v.toString();
    }

    static const Type &toQMLCompatibleType(const Type &v)
    {
        return v;
    }

    static void assignFromQmlType(Type &field, const Type &qmlValue)
    {
        field = qmlValue;
    }

};


template<typename Type>
struct TypeHandler<Type, typename std::enable_if<std::is_enum<Type>::value>::type>
{
    typedef Type QMLType;

    static void write(BinarySeralizer &msg, const Type &param)
    {
        msg << static_cast<int>(param);
    }

    static void read(BinarySeralizer &msg, Type &param)
    {
        int i;
        msg >> i;
        param = static_cast<Type>(i);
    }

    static QString toString(const Type &v)
    {
        return facelift::enumToString(v);
    }

    static Type fromVariant(const QVariant &variant)
    {
        return static_cast<Type>(variant.toInt());
    }

    static const Type &toQMLCompatibleType(const Type &v)
    {
        return v;
    }

    static void assignFromQmlType(Type &field, const Type &qmlValue)
    {
        field = qmlValue;
    }

    template<typename ReceiverType, typename Function>
    static void connectChangeSignals(const QVariant &variant, ReceiverType *receiver, Function function,
            QList<QMetaObject::Connection> &connections)
    {
        // nothing to connect
        Q_UNUSED(variant);
        Q_UNUSED(receiver);
        Q_UNUSED(function);
        Q_UNUSED(connections);
    }

};



template<>
struct TypeHandler<bool> : public TypeHandlerBase
{
    typedef bool QMLType;

    static QString toString(const bool &v)
    {
        return v ? "true" : "false";
    }

    static bool fromVariant(const QVariant &variant)
    {
        return variant.toBool();
    }

};


template<>
struct TypeHandler<int> : public TypeHandlerBase
{
    typedef int QMLType;

    static QString toString(const int &v)
    {
        return QString::number(v);
    }

    static int fromVariant(const QVariant &variant)
    {
        return variant.toInt();
    }

};


template<>
struct TypeHandler<float> : public TypeHandlerBase
{
    typedef float QMLType;

    static QString toString(const float &v)
    {
        return QString::number(v);
    }

    static float fromVariant(const QVariant &variant)
    {
        return variant.toFloat();
    }
};


template<>
struct TypeHandler<QString> : public TypeHandlerBase
{
    typedef QString QMLType;

    static QString toString(const QString &v)
    {
        QString s("\"");
        s += v;
        s += "\"";
        return s;
    }

    static QString fromVariant(const QVariant &variant)
    {
        return variant.toString();
    }

};


template<typename ElementType>
struct TypeHandler<QList<ElementType> >
{
    typedef QVariantList QMLType;

    static void write(BinarySeralizer &msg, const QList<ElementType> &list)
    {
        int count = list.size();
        msg << count;
        for (const auto &e : list) {
            TypeHandler<ElementType>::write(msg, e);
        }
    }

    static void read(BinarySeralizer &msg, QList<ElementType> &list)
    {
        list.clear();
        int count;
        msg >> count;
        for (int i = 0; i < count; i++) {
            ElementType e;
            TypeHandler<ElementType>::read(msg, e);
            list.append(e);
        }
    }

    static QString toString(const QList<ElementType> &v)
    {
        QString s;
        QTextStream str(&s);
        str << "[ ";
        for (const auto &element : v) {
            str << TypeHandler<ElementType>::toString(element);
            str << ", ";
        }
        str << "]";
        return s;
    }

    static QVariantList toQMLCompatibleType(const QList<ElementType> &list)
    {
        QVariantList variantList;
        for (const auto &e : list) {
            variantList.append(QVariant::fromValue(TypeHandler<ElementType>::toQMLCompatibleType(e)));
        }
        return variantList;
    }

    static void assignFromQmlType(QList<ElementType> &field, const QVariantList &qmlValue)
    {
        field.clear();
        for (const auto &v : qmlValue) {
            if (v.canConvert<ElementType>()) {
                ElementType element = v.value<ElementType>();
                field.append(element);
            } else {
                qFatal("Bad array item type");
            }
        }
    }

    template<typename ReceiverType, typename Function>
    static void connectChangeSignals(const QVariant &variant, ReceiverType *receiver, Function function,
            QList<QMetaObject::Connection> &connections)
    {
        // nothing to connect for most of the types
        Q_UNUSED(variant);
        Q_UNUSED(receiver);
        Q_UNUSED(function);
        Q_UNUSED(connections);
        Q_ASSERT(false);
    }

};

template<typename ElementType>
struct TypeHandler<QMap<QString, ElementType> >
{
    typedef QVariantMap QMLType;

    static void write(BinarySeralizer &msg, const QMap<QString, ElementType> &map)
    {
        int count = map.size();
        msg << count;
        for (auto i = map.constBegin(); i != map.constEnd(); ++i) {
            TypeHandler<QString>::write(msg, i.key());
            TypeHandler<ElementType>::write(msg, i.value());
        }
    }

    static void read(BinarySeralizer &msg, QMap<QString, ElementType> &map)
    {
        map.clear();
        int count;
        msg >> count;
        for (int i = 0; i < count; i++) {
            QString key;
            ElementType value;
            TypeHandler<QString>::read(msg, key);
            TypeHandler<ElementType>::read(msg, value);
            map.insert(key, value);
        }
    }

    static QString toString(const QMap<QString, ElementType> &map)
    {
        QString s;
        QTextStream str(&s);
        str << "[ ";
        for (auto i = map.constBegin(); i != map.constEnd(); ++i) {
            str << TypeHandler<QString>::toString(i.key());
            str << ":";
            str << TypeHandler<ElementType>::toString(i.value());
            str << ", ";
        }
        str << "]";
        return s;
    }

    static QVariantMap toQMLCompatibleType(const QMap<QString, ElementType> &map)
    {
        QVariantMap variantMap;
        for (auto i = map.constBegin(); i != map.constEnd(); ++i) {
            variantMap.insert(i.key(), QVariant::fromValue(TypeHandler<ElementType>::toQMLCompatibleType(i.value())));
        }

        return variantMap;
    }

    static void assignFromQmlType(QMap<QString, ElementType> &field, const QVariantMap &qmlValue)
    {
        field.clear();
        for (auto i = qmlValue.constBegin(); i != qmlValue.constEnd(); ++i) {
            QVariant value = i.value();
            if (value.canConvert<ElementType>()) {
                field.insert(i.key(), value.value<ElementType>());
            } else {
                qFatal("Bad map item value type");
            }
        }
    }
};


template<typename ... FieldTypes>
BinarySeralizer &operator<<(BinarySeralizer &stream, const Structure<FieldTypes ...> &s)
{
    for_each_in_tuple_const(s.asTuple(), StreamWriteFunction<BinarySeralizer>(stream));
    return stream;
}


template<typename ... FieldTypes>
BinarySeralizer &operator>>(BinarySeralizer &stream, Structure<FieldTypes ...> &s)
{
    for_each_in_tuple(s.asTuple(), StreamReadFunction<BinarySeralizer>(stream));
    return stream;
}

template<typename InterfaceType, typename PropertyType>
using PropertyGetter = const PropertyType &(*)();

class InterfaceBase;


class ServiceRegistry : public QObject
{
    Q_OBJECT

public:
    static ServiceRegistry &instance();

    virtual ~ServiceRegistry();

    void registerObject(InterfaceBase *i);

    const QList<InterfaceBase *> objects() const
    {
        return m_objects;
    }

    Q_SIGNAL void objectRegistered(InterfaceBase *object);
    Q_SIGNAL void objectDeregistered(InterfaceBase *object);

private:
    QList<InterfaceBase *> m_objects;

};

/**
 * Base interface which every interface inherits from
 */
class InterfaceBase : public QObject
{
    Q_OBJECT

public:
    typedef void QMLFrontendType;

    InterfaceBase(QObject *parent = nullptr) :
        QObject(parent)
    {
    }

    void setImplementationID(const QString &id)
    {
        m_implementationID = id;
    }

    const QString &implementationID() const
    {
        return m_implementationID;
    }

    virtual bool ready() const
    {
        return m_ready;
    }

    Q_SIGNAL void readyChanged();

    QObject *impl()
    {
        return this;
    }

    void init(const QString &interfaceName)
    {
        m_interfaceName = interfaceName;
        facelift::ServiceRegistry::instance().registerObject(this);
    }

    const QString &interfaceID()
    {
        return m_interfaceName;
    }

    Q_SIGNAL void componentCompleted();

protected:
    friend class ModelQMLImplementationBase;

    void setReady(bool ready)
    {
        if (m_ready != ready) {
            m_ready = ready;
            emit readyChanged();
        }
    }

private:
    QString m_implementationID = "Undefined";
    QString m_interfaceName;

    bool m_ready = true;

};

template<typename QMLType>
void qmlRegisterType(const char *uri, const char *typeName)
{
    ::qmlRegisterType<QMLType>(uri, 1, 0, typeName);
}

template<typename QMLType>
void qmlRegisterType(const char *uri)
{
    ::qmlRegisterType<QMLType>(uri, QMLType::INTERFACE_NAME);
}


template<typename ElementType>
inline QList<QVariant> toQListQVariant(const QList<ElementType> &list)
{
    QList<QVariant> variantList;
    for (const auto &e : list) {
        variantList.append(QVariant::fromValue(e));
    }
    return variantList;
}

template<typename ElementType>
inline QList<QVariant> toQListQVariantEnum(const QList<ElementType> &list)
{
    QList<QVariant> variantList;
    for (const auto &e : list) {
        variantList.append(QVariant::fromValue(e));
    }
    return variantList;
}

template<typename Type>
inline const typename TypeHandler<Type>::QMLType toQMLCompatibleType(const Type &v)
{
    return TypeHandler<Type>::toQMLCompatibleType(v);
}

template<typename Type, typename QmlType>
static void assignFromQmlType(Type &value, const QmlType &qmlValue)
{
    TypeHandler<Type>::assignFromQmlType(value, qmlValue);
}

template<typename Type, typename QmlType>
static Type toProviderCompatibleType(const QmlType &qmlValue)
{
    Type value;
    assignFromQmlType<Type, QmlType>(value, qmlValue);
    return value;
}

template<typename Class, typename PropertyType>
class PropertyInterface
{

public:
    typedef void (Class::*ChangeSignal)();
    typedef const PropertyType &(Class::*GetterMethod)() const;

    PropertyInterface(Class *o, GetterMethod g, ChangeSignal s)
    {
        object = o;
        signal = s;
        getter = g;
    }

    const PropertyType &value() const
    {
        const auto &v = (object->*getter)();
        return v;
    }

    Class *object;
    ChangeSignal signal;
    GetterMethod getter;

};


template<typename Class, typename PropertyType>
class ModelPropertyInterface
{

public:
    ModelPropertyInterface()
    {
    }

};


template<typename Class, typename ServiceType>
class ServicePropertyInterface
{
public:
    typedef void (Class::*ChangeSignal)();
    typedef ServiceType * (Class::*GetterMethod)();

    ServicePropertyInterface(Class *o, GetterMethod g, ChangeSignal s)
    {
        object = o;
        signal = s;
        getter = g;
    }

    ServiceType *value() const
    {
        return (object->*getter)();
    }

    Class *object;
    ChangeSignal signal;
    GetterMethod getter;

};



/**
 * Base class for all generated Module classes
 */
class ModuleBase
{

public:
    ModuleBase()
    {
    }

    static void registerQmlTypes(const char *uri, int majorVersion, int minorVersion);
};


class StructureFactoryBase : public QObject
{

    Q_OBJECT

public:
    StructureFactoryBase(QQmlEngine *engine) : QObject(engine)
    {
    }

    template<typename Type>
    static QObject *getter(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
    {
        Q_UNUSED(jsEngine);
        return new Type(qmlEngine);
    }

};


/**
 * This function simply calls Qt's qRegisterMetaType function
 */
template<typename Type>
inline int qRegisterMetaType()
{
    auto r = ::qRegisterMetaType<Type>();
    return r;
}


template<typename Type>
struct TypeHandler<Type *, typename std::enable_if<std::is_base_of<InterfaceBase, Type>::value>::type>
{
    typedef typename Type::QMLFrontendType* QMLType;

    static void write(BinarySeralizer &msg, const Type &param)
    {
        NOT_IMPLEMENTED();
    }

    static void read(BinarySeralizer &msg, Type &param)
    {
        NOT_IMPLEMENTED();
    }

    static Type *fromVariant(const QVariant &variant)
    {
        Q_UNUSED(variant);
        NOT_IMPLEMENTED();
        return nullptr;
    }

    static QString toString(const Type *v)
    {
        auto s = (size_t)(v);
        return QString::number(s, 16);
    }

    template<typename ReceiverType, typename Function>
    static void connectChangeSignals(const QVariant &variant, ReceiverType *receiver, Function function,
            QList<QMetaObject::Connection> &connections)
    {
        NOT_IMPLEMENTED();
        Q_UNUSED(variant);
        Q_UNUSED(receiver);
        Q_UNUSED(function);
        Q_UNUSED(connections);
    }

    static QMLType toQMLCompatibleType(Type *v)
    {
        return getQMLFrontend(v);
    }

};


template<typename Type>
inline QString toString(const Type &v)
{
    return TypeHandler<Type>::toString(v);
}

template<typename Type>
inline Type fromVariant(const QVariant &v)
{
    return TypeHandler<Type>::fromVariant(v);
}

template<typename Type>
inline const QList<Type> &validValues()
{
    return QList<Type>();
}


class ModelBase : public QObject
{

    Q_OBJECT

public:
    Q_SIGNAL void dataChanged(int first, int last);

    void dataChanged(int index)
    {
        dataChanged(index, index);
    }

    Q_SIGNAL void beginInsertElements(int first, int last);
    Q_SIGNAL void beginRemoveElements(int first, int last);

    Q_SIGNAL void endInsertElements();
    Q_SIGNAL void endRemoveElements();

    virtual int size() const = 0;

};


template<typename ElementType>
class Model : public ModelBase
{
public:
    virtual ElementType elementAt(int index) const = 0;

};


template<typename Type, typename Sfinae = void>
struct QMLModelTypeHandler
{
    static QJSValue toJSValue(const Type &v, QQmlEngine *engine)
    {
        return engine->toScriptValue(facelift::toQMLCompatibleType(v));
    }

    static void fromJSValue(Type &v, const QJSValue &value, QQmlEngine *engine)
    {
        v = engine->fromScriptValue<Type>(value);
    }

};


template<typename Type>
QJSValue toJSValue(const Type &v, QQmlEngine *engine)
{
    return QMLModelTypeHandler<Type>::toJSValue(v, engine);
}


template<typename Type>
void fromJSValue(Type &v, const QJSValue &jsValue, QQmlEngine *engine)
{
    QMLModelTypeHandler<Type>::fromJSValue(v, jsValue, engine);
}


template<typename CallBack>
class TAsyncAnswerMaster
{

public:
    TAsyncAnswerMaster(CallBack callback) : m_callback(callback)
    {
    }

    ~TAsyncAnswerMaster()
    {
        if (!m_isAlreadyAnswered) {
            qWarning() << "No answer provided to asynchronous call";
        }
    }

    template<typename ... Types>
    void call(const Types & ... args)
    {
        setAnswered();
        m_callback(args ...);
    }

private:
    void setAnswered()
    {
        Q_ASSERT(m_isAlreadyAnswered == false);
        m_isAlreadyAnswered = true;
    }

protected:
    bool m_isAlreadyAnswered = false;
    CallBack m_callback;

};


template<typename ReturnType>
class AsyncAnswer
{
    typedef std::function<void (const ReturnType &)> CallBack;

public:
    class Master : public TAsyncAnswerMaster<CallBack>
    {
    public:
        using TAsyncAnswerMaster<CallBack>::m_callback;
        Master(CallBack callback) : TAsyncAnswerMaster<CallBack>(callback)
        {
        }
    };

    AsyncAnswer()
    {
    }

    AsyncAnswer(CallBack callback) : m_master(new Master(callback))
    {
    }

    AsyncAnswer(const AsyncAnswer &other) : m_master(other.m_master)
    {
    }

    AsyncAnswer &operator=(const AsyncAnswer &other)
    {
        m_master = other.m_master;
        return *this;
    }

    void operator()(const ReturnType &returnValue)
    {
        m_master->call(returnValue);
    }

private:
    std::shared_ptr<Master> m_master;
};

template<>
class AsyncAnswer<void>
{
    typedef std::function<void ()> CallBack;

public:
    class Master : public TAsyncAnswerMaster<CallBack>
    {
    public:
        using TAsyncAnswerMaster<CallBack>::m_callback;

        Master(CallBack callback) : TAsyncAnswerMaster<CallBack>(callback)
        {
        }
    };

    AsyncAnswer()
    {
    }

    AsyncAnswer(CallBack callback) : m_master(new Master(callback))
    {
    }

    AsyncAnswer(const AsyncAnswer &other) : m_master(other.m_master)
    {
    }

    void operator()()
    {
        m_master->call();
    }

private:
    std::shared_ptr<Master> m_master;
};

}

template<typename ElementType>
inline QTextStream &operator<<(QTextStream &outStream, const facelift::Map<ElementType> &f)
{
    outStream << "[";
    for (const auto &e : f.toStdMap()) {
        outStream << e.first << "=" << e.second << ", ";
    }
    outStream << "]";
    return outStream;
}


Q_DECLARE_METATYPE(facelift::InterfaceBase *)
