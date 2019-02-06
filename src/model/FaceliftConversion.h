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

#pragma once

#include "FaceliftCommon.h"
#include "FaceliftModel.h"
#include "FaceliftUtils.h"
#include "StructureBase.h"

namespace facelift {



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


struct FaceliftModelLib_EXPORT BinarySeralizer
{
    BinarySeralizer(QByteArray &array) : stream(&array, QIODevice::WriteOnly)
    {
    }
    BinarySeralizer(const QByteArray &array) : stream(array)
    {
    }
    QDataStream stream;
};



template<typename Type>
struct TypeHandler<Type *, typename std::enable_if<std::is_base_of<InterfaceBase, Type>::value>::type>
{
    typedef typename Type::QMLFrontendType* QMLType;

    static void write(BinarySeralizer &msg, const Type* param)
    {
        Q_UNUSED(msg);
        Q_UNUSED(param);
        NOT_IMPLEMENTED();
    }

    static void read(BinarySeralizer &msg, Type* &param)
    {
        Q_UNUSED(msg);
        Q_UNUSED(param);
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

    static QVariant toQmlContainerElement(Type *v)
    {
        return QVariant::fromValue(toQMLCompatibleType(v));
    }
};


template<typename Type>
inline Type fromVariant(const QVariant &v)
{
    return TypeHandler<Type>::fromVariant(v);
}


struct FaceliftModelLib_EXPORT TypeHandlerBase
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

    template<typename Type>
    static QVariant toQmlContainerElement(const Type& v)
    {
        return QVariant::fromValue(v);
    }

    template<typename Type, typename QmlType>
    static void assignFromQmlType(Type &field, const QmlType &qmlValue)
    {
        field = qmlValue;
    }

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
struct StructTypeHandler
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

    static QString toString(const Type &v)
    {
        return v.toString();
    }

    static const Type &toQMLCompatibleType(const Type &v)
    {
        return v;
    }

    static QVariant toQmlContainerElement(const Type& v)
    {
        return QVariant::fromValue(v);
    }

    static void assignFromQmlType(Type &field, const Type &qmlValue)
    {
        field = qmlValue;
    }
};


// Specialization for structures which have QObjectWrapperType enabled
template<typename Type>
struct TypeHandler<Type, typename std::enable_if<Type::IsStructWithQObjectWrapperType>::type> : public StructTypeHandler<Type>
{
    static Type fromVariant(const QVariant &variant)
    {
        Type v;
        typedef typename Type::QObjectWrapperType QObjectWrapperType;

        if (variant.canConvert<QObjectWrapperType *>()) {
            auto qobjectWrapper = variant.value<QObjectWrapperType *>();
            v = qobjectWrapper->gadget();
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

};


template<typename Type>
struct TypeHandler<Type, typename std::enable_if<Type::IsStructWithoutQObjectWrapperType>::type> : public StructTypeHandler<Type>
{
    static Type fromVariant(const QVariant &variant)
    {
        Type v;
        if (variant.canConvert<Type>()) {
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
        M_UNUSED(variant, receiver, function, connections);
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

    static Type toQmlContainerElement(const Type &v)
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
            variantList.append(TypeHandler<ElementType>::toQmlContainerElement(e));
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
            variantMap.insert(i.key(), TypeHandler<ElementType>::toQmlContainerElement(i.value()));
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

template<typename Type>
inline const typename TypeHandler<Type>::QMLType toQMLCompatibleType(const Type &v)
{
    return TypeHandler<Type>::toQMLCompatibleType(v);
}

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
struct QMLModelTypeHandler<Type, typename std::enable_if<std::is_enum<Type>::value>::type>
{
    static QJSValue toJSValue(const Type &v, QQmlEngine *engine)
    {
        Q_UNUSED(engine)
        return QJSValue(v);
    }

    static void fromJSValue(Type &v, const QJSValue &value, QQmlEngine *engine)
    {
        Q_UNUSED(engine)
        v = static_cast<Type>(value.toInt());
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


inline void appendJSValue(QJSValueList& list, QQmlEngine * engine) {
    Q_UNUSED(list);
    Q_UNUSED(engine);
}

template<typename Arg1Type, typename  ... Args>
inline void appendJSValue(QJSValueList& list, QQmlEngine * engine,  const Arg1Type & arg1, const Args & ...args) {
    list.append(facelift::toJSValue(arg1, engine));
    appendJSValue(list, engine, args...);
}

template<typename  ... Args>
inline void callJSCallback(QQmlEngine* engine, QJSValue &callback, const Args & ...args)
{
    if (!callback.isUndefined()) {
        if (callback.isCallable()) {
            Q_ASSERT(engine != nullptr);
            QJSValueList jsList;
            appendJSValue(jsList, engine, args...);
            auto returnValue = callback.call(jsList);
            if (returnValue.isError()) {
                qCritical().noquote() << "Error executing JS callback. Error type:" << returnValue.property("name").toString()
                                      << "\nFile:" << returnValue.property("fileName").toString()
                                      << "\nLine:" << returnValue.property("lineNumber").toInt()
                                      << "\nMessage:" << returnValue.property("message").toString()
                                      << "\nStack trace:" << returnValue.property("stack").toString();
            }
        } else {
            qCritical("Provided JS object is not callable");
        }
    }
}



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


template<typename Type>
QByteArray serializeStructure(const Type& o)
{
    QByteArray array;
    BinarySeralizer ds(array);
    ds << o;
    return array;
}

template<typename Type>
void deserializeStructure(Type& o, const QByteArray &array)
{
    BinarySeralizer ds(array);
    ds >> o;
}



}
