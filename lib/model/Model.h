/*
 *   This is part of the QMLCppAPI project
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

#include <array>

#include "../utils.h"


#define STRINGIFY_(x) # x
#define STRINGIFY(x) STRINGIFY_(x)

typedef int ModelElementID;

class ModelStructure
{
public:
    Q_GADGET

public:
    static constexpr int ROLE_ID = 1000;
    static constexpr int ROLE_BASE = ROLE_ID + 1;

    //    Q_PROPERTY(int id READ id CONSTANT)

    ModelElementID id() const
    {
        return m_id;
    }

    ModelStructure()
    {
        m_id = s_nextID++;
    }

    void setId(ModelElementID id)
    {
        m_id = id;
    }

protected:
    ModelElementID m_id;

private:
    static ModelElementID s_nextID;

};

template<typename Type>
inline const QList<Type> &validValues()
{
}

template<typename Type>
inline QVariant toVariant(const Type &v)
{
    return v;
}

template<typename Type>
inline QVariant toVariant(const QList<Type> &v)
{
    Q_ASSERT(false);
    Q_UNUSED(v);
    return "";
}

template<typename Type>
inline QString toString(const Type &v)
{
    Q_UNUSED(v);
    Q_ASSERT(false);
    return "Unknown";
}

template<>
inline QString toString(const int &v)
{
    return QString::number(v);
}

template<>
inline QString toString(const bool &v)
{
    return v ? "true" : "false";
}

template<>
inline QString toString(const QString &v)
{
    return v;
}

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
struct ModelTypeTraits<StructType, typename std::enable_if<std::is_base_of<ModelStructure, StructType>::value>::type>
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


template<typename ... FieldTypes>
class TModelStructure :
    public ModelStructure
{

public:
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

    QByteArray serialize() const
    {
        QByteArray array;
        BinarySeralizer ds(array);
        ds << *this;
        return array;
    }

    void deserialize(const QByteArray &array)
    {
        BinarySeralizer ds(array);
        ds >> *this;
    }

    void setValue(FieldTupleTypes value)
    {
        m_values = value;
    }

    void copyFrom(const TModelStructure &other)
    {
        setValue(other.m_values);
        m_id = other.id();
    }

    bool operator==(const TModelStructure &right) const
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
    toString__(const std::tuple<Tp ...> &t, const FieldNames &names, QTextStream &outStream) const
    {
        Q_UNUSED(t);
        Q_UNUSED(names);
        Q_UNUSED(outStream);
    }

    template<std::size_t I = 0, typename ... Tp>
    inline typename std::enable_if < I<sizeof ... (Tp), void>::type
    toString__(const std::tuple<Tp ...> &t, const FieldNames &names, QTextStream &outStream) const
    {
        outStream << names[I] << "=" << toString(std::get<I>(t));
        if (I != FieldCount) {
            outStream << ", ";
        }
        toString__<I + 1, Tp ...>(t, names, outStream);
    }

    QString toStringWithFields(const FieldNames &names) const
    {
        Q_UNUSED(names);
        QString s;
        QTextStream outStream(&s);
        toString__(m_values, names, outStream);
        return s;
    }

    FieldTupleTypes m_values = {};

};




template<typename Type, typename Enable = void>
struct BinarySerializationTypeHandler
{
    static void write(BinarySeralizer &msg, const Type &v)
    {
        msg.stream << v;
    }

    static void read(BinarySeralizer &msg, Type &v)
    {
        msg.stream >> v;
    }
};


template<typename Type>
BinarySeralizer &operator<<(BinarySeralizer &msg, const Type &v)
{
    BinarySerializationTypeHandler<Type>::write(msg, v);
    return msg;
}


template<typename Type>
BinarySeralizer &operator>>(BinarySeralizer &msg, Type &v)
{
    BinarySerializationTypeHandler<Type>::read(msg, v);
    return msg;
}

template<typename Type>
struct BinarySerializationTypeHandler<Type, typename std::enable_if<std::is_base_of<ModelStructure, Type>::value>::type>
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

};

template<typename Type>
struct BinarySerializationTypeHandler<Type, typename std::enable_if<std::is_enum<Type>::value>::type>
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
};


template<typename ElementType>
struct BinarySerializationTypeHandler<QList<ElementType> >
{
    static void write(BinarySeralizer &msg, const QList<ElementType> &list)
    {
        int count = list.size();
        msg << count;
        for (const auto &e : list) {
            BinarySerializationTypeHandler<ElementType>::write(msg, e);
        }
    }

    static void read(BinarySeralizer &msg, QList<ElementType> &list)
    {
        list.clear();
        int count;
        msg >> count;
        for (int i = 0; i < count; i++) {
            ElementType e;
            BinarySerializationTypeHandler<ElementType>::read(msg, e);
            list.append(e);
        }
    }

};


template<typename ... FieldTypes>
BinarySeralizer &operator<<(BinarySeralizer &stream, const TModelStructure<FieldTypes ...> &s)
{
    for_each_in_tuple_const(s.asTuple(), StreamWriteFunction<BinarySeralizer>(stream));
    return stream;
}


template<typename ... FieldTypes>
BinarySeralizer &operator>>(BinarySeralizer &stream, TModelStructure<FieldTypes ...> &s)
{
    for_each_in_tuple(s.asTuple(), StreamReadFunction<BinarySeralizer>(stream));
    return stream;
}


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

    QObject *impl()
    {
        Q_ASSERT(this != nullptr);
        return this;
    }

private:
    QString m_implementationID = "Undefined";
};


class ModelQMLImplementationBase :
    public QQuickItem
{

    Q_OBJECT

public:
    Q_PROPERTY(QString implementationID READ implementationID WRITE setImplementationID)

    ModelQMLImplementationBase(QQuickItem *parent = nullptr) :
        QQuickItem(parent)
    {
    }

    void setImplementationID(const QString &id)
    {
        Q_ASSERT(m_interface != nullptr);
        m_interface->setImplementationID(id);
    }

    const QString &implementationID() const
    {
        Q_ASSERT(m_interface != nullptr);
        return m_interface->implementationID();
    }

    void setInterface(InterfaceBase *interface)
    {
        m_interface = interface;
    }

    InterfaceBase *m_interface = nullptr;
};

template<typename InterfaceType>
class ModelQMLImplementation :
    public ModelQMLImplementationBase
{

public:
    ModelQMLImplementation(QQuickItem *parent = nullptr) :
        ModelQMLImplementationBase(parent)
    {
    }

    static QString &modelImplementationFilePath()
    {
        static QString s_modelImplementationFilePath;
        return s_modelImplementationFilePath;
    }

    void retrieveFrontend()
    {
        m_interface = retrieveFrontendUnderConstruction();
        setInterface(m_interface);
    }

    static void registerTypes(const char *theURI)
    {
        typedef typename InterfaceType::QMLImplementationModelType QMLImplementationModelType;

        // Register the component used to actually implement the model in QML
        // the QML file containing the model implementation should have this type at its root
        qmlRegisterType<QMLImplementationModelType>(theURI, 1, 0, QMLImplementationModelType::QML_NAME);
    }

    static InterfaceType *retrieveFrontendUnderConstruction()
    {
        Q_ASSERT(frontendUnderConstruction() != nullptr);
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
    InterfaceType *m_interface = nullptr;

};

template<typename QMLType>
void qmlRegisterType(const char *uri, const char *typeName)
{
    qmlRegisterType<QMLType>(uri, 1, 0, typeName);
}

template<typename QMLType>
void qmlRegisterType(const char *uri)
{
    qmlRegisterType<QMLType>(uri, QMLType::INTERFACE_NAME);
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
