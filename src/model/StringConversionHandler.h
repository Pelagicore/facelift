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

#include <QString>
#include <QMap>
#include <QTextStream>
#include <QVariant>
#include <QList>
#include <type_traits>
#include "FaceliftCommon.h"

namespace facelift {

class InterfaceBase;
class StructureBase;

template<typename Type, typename Enable = void>
struct StringConversionHandler
{
    typedef Type QMLType;

    static QString toString(const Type &v)
    {
        QString s;
        QTextStream(&s) << v;
        return s;
    }

};

template<>
struct StringConversionHandler<QVariant>
{
    static QString toString(const QVariant &variant) {
        return variant.toString();
    }
};

QString qObjectToString(const QObject *o);

template<typename Type>
struct StringConversionHandler<Type *, typename std::enable_if<std::is_base_of<InterfaceBase, Type>::value>::type>
{
    static QString toString(const Type *o)
    {
        return qObjectToString(o);
    }
};


template<typename Type>
struct StringConversionHandler<Type, typename std::enable_if<std::is_base_of<StructureBase, Type>::value>::type>
{
    static QString toString(const Type &v)
    {
        return v.toString();
    }
};


template<typename Type>
struct StringConversionHandler<Type, typename std::enable_if<std::is_enum<Type>::value>::type>
{
    static QString toString(const Type &v)
    {
        return facelift::enumToString(v);
    }
};

template<typename ElementType>
struct StringConversionHandler<QList<ElementType> >
{
    static QString toString(const QList<ElementType> &v)
    {
        QString s;
        QTextStream str(&s);
        str << "[ ";
        for (const auto &element : v) {
            str << StringConversionHandler<ElementType>::toString(element);
            str << ", ";
        }
        str << "]";
        return s;
    }
};

template<typename ElementType>
struct StringConversionHandler<QMap<QString, ElementType> >
{
    static QString toString(const QMap<QString, ElementType> &map)
    {
        QString s;
        QTextStream str(&s);
        str << "[ ";
        for (auto i = map.constBegin(); i != map.constEnd(); ++i) {
            str << StringConversionHandler<QString>::toString(i.key());
            str << ":";
            str << StringConversionHandler<ElementType>::toString(i.value());
            str << ", ";
        }
        str << "]";
        return s;
    }
};

template<typename Type>
inline QString toString(const Type &v)
{
    return StringConversionHandler<Type>::toString(v);
}

}
