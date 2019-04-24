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

#if defined(FaceliftIPCCommonLib_LIBRARY)
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_IMPORT
#endif

#include "ipc-common.h"

namespace facelift {

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

struct FaceliftIPCCommonLib_EXPORT AppendDBUSSignatureFunction
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

template<typename ParentType>
struct SerializeParameterFunction
{
    SerializeParameterFunction(OutputPayLoad &msg, const ParentType &parent) :
        m_msg(msg),
        m_parent(parent)
    {
    }

    OutputPayLoad &m_msg;
    const ParentType &m_parent;

    template<typename Type>
    void operator()(const Type &v)
    {
        IPCTypeHandler<typename IPCTypeRegisterHandler<Type>::SerializedType>::write(m_msg,
                IPCTypeRegisterHandler<Type>::convertToSerializedType(v, m_parent));
    }
};


class FaceliftIPCCommonLib_EXPORT DBusSignatureHelper {

public:

template<size_t I = 0, typename ... Ts>
typename std::enable_if<I == sizeof ... (Ts)>::type
static appendDBUSMethodArgumentsSignature(QTextStream &s, std::tuple<Ts ...> &t, const std::array<const char *,
        sizeof ... (Ts)> &argNames)
{
    Q_UNUSED(s);
    Q_UNUSED(t);
    Q_UNUSED(argNames);
}

template<size_t I = 0, typename ... Ts>
typename std::enable_if < I<sizeof ... (Ts)>::type
static appendDBUSMethodArgumentsSignature(QTextStream &s, std::tuple<Ts ...> &t, const std::array<const char *,
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
static appendDBUSSignalArgumentsSignature(QTextStream &s, std::tuple<Ts ...> &t, const std::array<const char *,
        sizeof ... (Ts)> &argNames)
{
    Q_UNUSED(s);
    Q_UNUSED(t);
    Q_UNUSED(argNames);
}

template<size_t I = 0, typename ... Ts>
typename std::enable_if < I<sizeof ... (Ts)>::type
static appendDBUSSignalArgumentsSignature(QTextStream &s, std::tuple<Ts ...> &t, const std::array<const char *,
        sizeof ... (Ts)> &argNames)
{
    using Type = decltype(std::get<I>(t));
    s << "<arg name=\"" << argNames[I] << "\" type=\"";
    IPCTypeHandler<Type>::writeDBUSSignature(s);
    s << "\"/>";
    appendDBUSSignalArgumentsSignature<I + 1>(s, t, argNames);
}

template<size_t I = 0, typename ... Ts>
typename std::enable_if<I == sizeof ... (Ts)>::type
static appendDBUSTypeSignature(QTextStream &s, std::tuple<Ts ...> &t)
{
    Q_UNUSED(s);
    Q_UNUSED(t);
}

template<size_t I = 0, typename ... Ts>
typename std::enable_if < I<sizeof ... (Ts)>::type
static appendDBUSTypeSignature(QTextStream &s, std::tuple<Ts ...> &t)
{
    using Type = decltype(std::get<I>(t));
    IPCTypeHandler<Type>::writeDBUSSignature(s);
    appendDBUSTypeSignature<I + 1>(s, t);
}

template<typename Type>
static void addPropertySignature(QTextStream &s, const char *propertyName, bool isReadonly)
{
    s << "<property name=\"" << propertyName << "\" type=\"";
    std::tuple<Type> dummyTuple;
    appendDBUSTypeSignature(s, dummyTuple);
    s << "\" access=\"" << (isReadonly ? "read" : "readwrite") << "\"/>";
}

template<typename ... Args>
static void addMethodSignature(QTextStream &s, const char *methodName,
        const std::array<const char *, sizeof ... (Args)> &argNames)
{
    s << "<method name=\"" << methodName << "\">";
    std::tuple<Args ...> t;  // TODO : get rid of the tuple
    appendDBUSMethodArgumentsSignature(s, t, argNames);
    s << "</method>";
}

template<typename ... Args>
static void addSignalSignature(QTextStream &s, const char *methodName,
        const std::array<const char *, sizeof ... (Args)> &argNames)
{
    s << "<signal name=\"" << methodName << "\">";
    std::tuple<Args ...> t;  // TODO : get rid of the tuple
    appendDBUSSignalArgumentsSignature(s, t, argNames);
    s << "</signal>";
}

};

}
