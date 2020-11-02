/**********************************************************************
**
** Copyright (C) 2020 Luxoft Sweden AB
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

#include "TypeHandlerBase.h"
#include "StructTypeHandler.h"

namespace facelift {

#define NOT_IMPLEMENTED() qFatal("Not implemented")

template<typename Type> QString typeName()
{
    Type t = {};
    auto v = QVariant::fromValue(t);
    return v.typeName();
}

template<typename Type>
struct TypeHandler<Type *, typename std::enable_if<std::is_base_of<InterfaceBase, Type>::value>::type>
{
    typedef typename Type::QMLAdapterType* QMLType;

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
        return getQMLAdapter(v);
    }

    static QVariant toQmlContainerElement(Type *v)
    {
        return QVariant::fromValue(toQMLCompatibleType(v));
    }
};

// Specialization for structures which have QObjectWrapperType enabled
template<typename Type>
struct TypeHandler<Type, typename std::enable_if<Type::IsStructWithQObjectWrapperType>::type> : public StructTypeHandler<Type>
{
    static Type fromVariant(const QVariant &variant)
    {
        Type v {};
        typedef typename Type::QObjectWrapperType QObjectWrapperType;

        if (variant.canConvert<QObjectWrapperType *>()) {
            auto qobjectWrapper = variant.value<QObjectWrapperType *>();
            v = qobjectWrapper->gadget();
        } else if (variant.canConvert<Type>()) {
            v = variant.value<Type>();
        } else {
            faceliftSeriousError("Bad argument: %s / type: %s / expected type: %s", qPrintable(variant.toString()), variant.typeName(), qPrintable(typeName<Type>()));
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
            faceliftSeriousError("Bad argument: %s / type: %s / expected type: %s", qPrintable(variant.toString()), variant.typeName(), qPrintable(typeName<Type>()));
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

    static bool fromVariant(const QVariant &variant)
    {
        return variant.toBool();
    }

};


template<>
struct TypeHandler<int> : public TypeHandlerBase
{
    typedef int QMLType;

    static int fromVariant(const QVariant &variant)
    {
        return variant.toInt();
    }

};


template<>
struct TypeHandler<double> : public TypeHandlerBase
{
    typedef double QMLType;

    static double fromVariant(const QVariant &variant)
    {
        return variant.toDouble();
    }
};


template<>
struct TypeHandler<QString> : public TypeHandlerBase
{
    typedef QString QMLType;

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
                faceliftSeriousError("Bad array item: %s / type: %s / expected type: %s", qPrintable(v.toString()), qPrintable(v.typeName()), qPrintable(typeName<ElementType>()));
            }
        }
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
                faceliftSeriousError("Bad map item value: %s / type: %s / expected type: %s", qPrintable(value.toString()), qPrintable(value.typeName()), qPrintable(typeName<ElementType>()));
            }
        }
    }
};


}
