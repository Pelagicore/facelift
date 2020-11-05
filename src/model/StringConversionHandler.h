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
#include "FaceliftEnum.h"

namespace facelift {

class InterfaceBase;
class StructureBase;

template<typename T, typename Enable = void>
struct StringConversionHandler
{
    static QString toString(const T &v)
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

QString qObjectToString(const QObject *object);

template<typename T>
struct StringConversionHandler<T*, std::enable_if_t<std::is_base_of<InterfaceBase, T>::value>>
{
    static QString toString(const T *object)
    {
        return qObjectToString(object);
    }
};


template<typename T>
struct StringConversionHandler<T, std::enable_if_t<std::is_base_of<StructureBase, T>::value>>
{
    static QString toString(const T &value)
    {
        return value.toString();
    }
};

template<typename T>
struct StringConversionHandler<T, std::enable_if_t<QtPrivate::IsQEnumHelper<T>::Value, T>>
{
    static QString toString(T value)
    {
        return Enum::toString(value);        
    }
};

template<typename T>
struct StringConversionHandler<QList<T> >
{
    static QString toString(const QList<T> &v)
    {
        QString s;
        QTextStream str(&s);
        str << "[ ";
        for (const auto &element : v) {
            str << StringConversionHandler<T>::toString(element);
            str << ", ";
        }
        str << "]";
        return s;
    }
};

template<typename T>
struct StringConversionHandler<QMap<QString, T> >
{
    static QString toString(const QMap<QString, T> &map)
    {
        QString s;
        QTextStream str(&s);
        str << "[ ";
        for (auto i = map.constBegin(); i != map.constEnd(); ++i) {
            str << StringConversionHandler<QString>::toString(i.key());
            str << ":";
            str << StringConversionHandler<T>::toString(i.value());
            str << ", ";
        }
        str << "]";
        return s;
    }
};

template<typename T>
inline QString toString(const T &v)
{
    return StringConversionHandler<T>::toString(v);
}


}
