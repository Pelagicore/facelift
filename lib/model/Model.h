/*
 *   This is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 *   Please see the LICENSE file for details.
 */

#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <QQmlListProperty>
#include <QDebug>
#include <QQuickItem>
#include <QQmlEngine>
#include <QJSValue>
#include <QTimer>

#include <array>

#include "utils.h"


#define STRINGIFY_(x) # x
#define STRINGIFY(x) STRINGIFY_(x)

typedef int ModelElementID;

namespace facelift {

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
    NOT_IMPLEMENTED();
    return "\"No string representation\"";
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

};

template<typename Type, typename Enable = void>
struct TypeHandler
{
    static QString toString(const Type &v)
    {
        Q_UNUSED(v);
        return "Unknown";
    }

};

template<typename Type, typename Sfinae = void>
struct ModelTypeTraits
{
    typedef bool IsSimple;

    template<typename Type2>
    static void assignToVariant(const Type2 &value, QVariant &variant)
    {
        Q_UNUSED(value);
        Q_UNUSED(variant);
        variant = toVariant(value);
    }

};


template<typename StructType>
struct ModelTypeTraits<StructType, typename std::enable_if<std::is_base_of<StructureBase, StructType>::value>::type>
{
    typedef bool IsStruct;

    template<typename Type2>
    static void assignToVariant(const Type2 &value, QVariant &variant)
    {
        // We are not able to assign a structure type to a QVariant yet
        qFatal("Unable to access a structure member from a model");
        Q_UNUSED(value);
        Q_UNUSED(variant);
    }
};


template<typename EnumType>
struct ModelTypeTraits<EnumType, typename std::enable_if<std::is_enum<EnumType>::value>::type>
{
    typedef bool IsEnum;
};




template<typename ... FieldTypes>
class Structure :
    public StructureBase
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

    template<std::size_t I = 0, typename ... Tp>
    inline typename std::enable_if<I == sizeof ... (Tp), void>::type
    toVariant(const std::tuple<Tp ...> &t, QVariant &variant, size_t index) const
    {
        Q_UNUSED(t);
        Q_UNUSED(variant);
        Q_UNUSED(index);
    }

    template<std::size_t I = 0, typename ... Tp>
    inline typename std::enable_if < I<sizeof ... (Tp), void>::type
    toVariant(const std::tuple<Tp ...> &t, QVariant &variant, size_t index) const
    {
        typedef ModelTypeTraits<typeof(std::get<I>(t))> Trait;
        if (index == I + ROLE_BASE) {
            Trait::assignToVariant(std::get<I>(t), variant);
        }
        toVariant<I + 1, Tp ...>(t, variant, index);
    }

    QVariant getFieldAsVariant(int role) const
    {
        if (role == ROLE_ID) {
            return id();
        }

        QVariant v;
        toVariant(m_values, v, role);
        return v;
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

    static QHash<int, QByteArray> roleNames_(const FieldNames &fieldNames)
    {
        QHash<int, QByteArray> roleNames;
        roleNames[ROLE_ID] = "id";
        int i = ROLE_BASE;
        for (auto &fieldName : fieldNames) {
            roleNames[i++] = fieldName;
        }
        return roleNames;
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

};


template<typename Type>
struct TypeHandler<Type, typename std::enable_if<std::is_enum<Type>::value>::type>
{
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

};



template<>
struct TypeHandler<bool> :
    public TypeHandlerBase
{
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
struct TypeHandler<int> :
    public TypeHandlerBase
{
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
struct TypeHandler<float> :
    public TypeHandlerBase
{
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
struct TypeHandler<QString> :
    public TypeHandlerBase
{
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


class ServiceRegistry :
    public QObject
{
    Q_OBJECT

public:
    static ServiceRegistry &instance()
    {
        static ServiceRegistry reg;
        return reg;
    }

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

class IPCAdapterBase
{

};

/**
 * Base interface which every interface inherits from
 */
class InterfaceBase :
    public QObject
{
    Q_OBJECT

public:
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
            readyChanged();
        }
    }

private:
    QString m_implementationID = "Undefined";
    QString m_interfaceName;

    bool m_ready = true;

};


class ModelQMLImplementationBase :
    public QObject, public QQmlParserStatus
{
    Q_OBJECT

public:
    // We set a default property so that we can have children QML elements in our QML implementations, such as Timer
    Q_PROPERTY(QQmlListProperty<QObject> childItems READ childItems)
    Q_CLASSINFO("DefaultProperty", "childItems")

    ModelQMLImplementationBase(QObject *parent = nullptr) :
        QObject(parent)
    {
    }

    Q_PROPERTY(bool ready READ ready WRITE setReady NOTIFY readyChanged)

    void setReady(bool ready)
    {
        Q_ASSERT(m_interface != nullptr);
        m_interface->setReady(ready);
    }

    bool ready() const
    {
        Q_ASSERT(m_interface != nullptr);
        return m_interface->ready();
    }

    Q_SIGNAL void readyChanged();

    Q_PROPERTY(QString implementationID READ implementationID WRITE setImplementationID NOTIFY implementationIDChanged)

    void setImplementationID(const QString &id)
    {
        m_implementationID = id;
        assignImplementationID();
    }

    const QString &implementationID() const
    {
        return m_implementationID;
        Q_ASSERT(m_interface != nullptr);
        return m_interface->implementationID();
    }

    Q_SIGNAL void implementationIDChanged();

    void setInterface(InterfaceBase *interface)
    {
        m_interface = interface;
        connect(interface, &InterfaceBase::readyChanged, this, &ModelQMLImplementationBase::readyChanged);

        assignImplementationID();
        connect(this, &QObject::objectNameChanged, this, &ModelQMLImplementationBase::assignImplementationID);
    }

    QJSValue &checkMethod(QJSValue &method, const char *methodName)
    {
        if (!method.isCallable()) {
            qFatal("Method \"%s\" of Facelift interface implementation \"%s\" is about to be called but it is not implemented in your QML file. "
                    "That method MUST be implemented if it is called.", qPrintable(methodName), qPrintable(interface()->interfaceID()));
        }

        return method;
    }

    QQmlListProperty<QObject> childItems()
    {
        return QQmlListProperty<QObject>(this, m_children);
    }

    void classBegin() override
    {
    }

    void componentComplete() override
    {
        assignImplementationID();
        m_interface->componentCompleted();
    }

    InterfaceBase *interface() const
    {
        return m_interface;
    }

    void assignImplementationID()
    {
        Q_ASSERT(m_interface != nullptr);
        QString id;
        QTextStream s(&id);

        s << "QML implementation - " << metaObject()->className();
        if (!objectName().isEmpty()) {
            s << ", name = " << objectName();
        }
        s << "" << this;

        m_interface->setImplementationID(id);
    }

private:
    QList<QObject *> m_children;
    InterfaceBase *m_interface = nullptr;
    QString m_implementationID;
};

template<typename InterfaceType>
class ModelQMLImplementation :
    public ModelQMLImplementationBase
{

public:
    ModelQMLImplementation(QObject *parent = nullptr) :
        ModelQMLImplementationBase(parent)
    {
    }

    static void setModelImplementationFilePath(QString path)
    {
        modelImplementationFilePath() = path;
    }

    static QString &modelImplementationFilePath()
    {
        static QString s_modelImplementationFilePath;
        return s_modelImplementationFilePath;
    }

    virtual void initProvider(InterfaceType *provider) = 0;

    void retrieveFrontend()
    {
        m_interface = retrieveFrontendUnderConstruction();
        if (m_interface == nullptr) {
            m_interface = createFrontend();
        }

        Q_ASSERT(m_interface);

        initProvider(m_interface);
        setInterface(m_interface);
    }

    virtual InterfaceType *createFrontend() = 0;

    static void registerTypes(const char *theURI)
    {
        typedef typename InterfaceType::QMLImplementationModelType QMLImplementationModelType;

        // Register the component used to actually implement the model in QML
        // the QML file containing the model implementation should have this type at its root
        qmlRegisterType<QMLImplementationModelType>(theURI, 1, 0, QMLImplementationModelType::QML_NAME);
    }

    static InterfaceType *retrieveFrontendUnderConstruction()
    {
        auto instance = frontendUnderConstruction();
        frontendUnderConstruction() = nullptr;
        return instance;
    }

    static void setFrontendUnderConstruction(InterfaceType *instance)
    {
        Q_ASSERT(frontendUnderConstruction() == nullptr);
        frontendUnderConstruction() = instance;
    }

    template<typename ModelImplClass>
    static ModelImplClass *createComponent(QQmlEngine *engine, InterfaceType *frontend)
    {
        auto path = modelImplementationFilePath();

        // Save the reference to the frontend which we are currently creating, so that the QML model implementation is able
        // to access it from its constructor
        setFrontendUnderConstruction(frontend);
        ModelImplClass *r = nullptr;
        qDebug() << "Creating QML component from file : " << path;
        QQmlComponent component(engine, QUrl::fromLocalFile(path));
        if (!component.isError()) {
            QObject *object = component.create();
            r = qobject_cast<ModelImplClass *>(object);
        } else {
            qWarning() << "Error : " << component.errorString();
            qFatal("Can't create QML model");
        }
        return r;
    }

    void checkInterface() const
    {
        Q_ASSERT(m_interface != nullptr);
    }

private:
    static InterfaceType * &frontendUnderConstruction()
    {
        static InterfaceType *i = nullptr;
        return i;
    }

protected:
    InterfaceType *provider() const
    {
        return m_interface;
    }

    InterfaceType *m_interface = nullptr;

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

template<typename ElementType>
inline QList<QVariant> toQMLCompatibleType(const QList<ElementType> &list)
{
    QList<QVariant> variantList;
    for (const auto &e : list) {
        variantList.append(QVariant::fromValue(e));
    }
    return variantList;
}


template<typename Class, typename PropertyType>
class PropertyInterface
{

public:
    typedef void (Class::*ChangeSignal)();
    typedef const PropertyType & (Class::*GetterMethod)() const;

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


template<typename Class, typename PropertyType>
class ServicePropertyInterface
{
    // TODO : implement
public:
    ServicePropertyInterface()
    {
    }

};


class ServiceWrapperBase
{

protected:
    void addConnection(QMetaObject::Connection connection)
    {
        m_connections.append(connection);
    }

    void reset()
    {
        for (const auto &connection : m_connections) {
            auto successfull = QObject::disconnect(connection);
            Q_ASSERT(successfull);
        }
    }

    QVector<QMetaObject::Connection> m_connections;  /// The list of connections which this property is bound to

};

/**
 *
 */
template<typename WrappedType>
class ServiceWrapper :
    public WrappedType, public ServiceWrapperBase
{

public:
    void setWrapped(WrappedType *wrapped)
    {
        m_wrapped = wrapped;

        if (!m_wrapped.isNull()) {
            reset();
        }

        initConnections();
    }

    virtual ~ServiceWrapper()
    {
    }

protected:
    ServiceWrapper(QObject *parent) : WrappedType(parent)
    {
    }

    WrappedType *wrapped() const
    {
        Q_ASSERT(!m_wrapped.isNull());
        return m_wrapped.data();
    }

    virtual void initConnections(WrappedType *wrapped) = 0;

private:
    QPointer<WrappedType> m_wrapped;

};

class ModuleBase
{
public:
    ModuleBase()
    {
    }

};


template<typename Type>
inline int qRegisterMetaType()
{
    auto r = ::qRegisterMetaType<Type>();
    return r;
}

/*
template<typename Type>
inline QVariant toVariant(const Type &v)
{
    return v;
}

template<typename Type>
inline QVariant toVariant(const QList<Type> &v)
{
    NOT_IMPLEMENTED();
    Q_UNUSED(v);
    return "";
}
*/


template<typename Type>
struct TypeHandler<Type *, typename std::enable_if<std::is_base_of<InterfaceBase, Type>::value>::type>
{
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
        NOT_IMPLEMENTED();
        return nullptr;
    }

    static QString toString(const Type *v)
    {
        auto s = (size_t)(v);
        return QString::number(s, 16);
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

}


Q_DECLARE_METATYPE(facelift::InterfaceBase *)
