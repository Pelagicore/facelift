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

#if defined(FaceliftModelLib_LIBRARY)
#  define FaceliftModelLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftModelLib_EXPORT Q_DECL_IMPORT
#endif

#include "FaceliftCommon.h"
#include <QObject>
#include <QTextStream>
#include <QDBusArgument>

template<typename T, typename TEnum = void>
class QDBusEnumMarshal;

template<typename T>
class QDBusEnumMarshal<T, typename std::enable_if<std::is_enum<T>::value>::type>
{
public:
    static QDBusArgument& marshal(QDBusArgument &argument, const T& source)
    {
        argument.beginStructure();
        argument << static_cast<int>(source);
        argument.endStructure();
        return argument;
    }

    static const QDBusArgument& unmarshal(const QDBusArgument &argument, T &source)
    {
        int a;
        argument.beginStructure();
        argument >> a;
        argument.endStructure();

        source = static_cast<T>(a);

        return argument;
    }
};

template<typename T>
QDBusArgument& operator<<(QDBusArgument &argument, const T& source)
{
    return QDBusEnumMarshal<T>::marshal(argument, source);
}

template<typename T>
const QDBusArgument& operator>>(const QDBusArgument &argument, T &source)
{
    return QDBusEnumMarshal<T>::unmarshal(argument, source);
}

namespace facelift {

void onAssignFromStringError(const QString &s);

}
