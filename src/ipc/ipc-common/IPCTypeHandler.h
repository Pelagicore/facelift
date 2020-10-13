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

#include "ipc-common.h"
#include "ModelProperty.h"
#include "OutputPayLoad.h"
#include "InputPayLoad.h"
#include "AppendDBUSSignatureFunction.h"

namespace facelift {

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
struct IPCTypeHandler<double>
{
    static void writeDBUSSignature(QTextStream &s)
    {
        s << "d";
    }

    static void write(OutputPayLoad &msg, const double &v)
    {
        msg.writeSimple(v);
    }

    static void read(InputPayLoad &msg, double &v)
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
        IPCTypeHandler<typename Type::FieldTupleTypes>::write(msg, param.asTuple());
    }

    static void read(InputPayLoad &msg, Type &param)
    {
        IPCTypeHandler<typename Type::FieldTupleTypes>::read(msg, param.asTuple());
    }

};


template<typename ... Types>
struct IPCTypeHandler<std::tuple<Types...>>
{
    static void write(OutputPayLoad &msg, const std::tuple<Types...> &param)
    {
        for_each_in_tuple_const(param, StreamWriteFunction<OutputPayLoad>(msg));
    }

    static void read(InputPayLoad &msg, std::tuple<Types...> &param)
    {
        for_each_in_tuple(param, StreamReadFunction<InputPayLoad>(msg));
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
struct IPCTypeHandler<QList<ElementType>>
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
struct IPCTypeHandler<QMap<QString, ElementType>>
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

}
