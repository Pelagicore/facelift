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

#include <type_traits>
#include <initializer_list>
#include <QString>
#include <QLoggingCategory>

#if defined(FaceliftModelLib_LIBRARY)
#  define FaceliftModelLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftModelLib_EXPORT Q_DECL_IMPORT
#endif

#define STRINGIFY_(x) # x
#define STRINGIFY(x) STRINGIFY_(x)

/**
 * Avoid "unused parameter" warnings, with multiple arguments
 */
template<typename ... Args>
void M_UNUSED(const Args & ... args)
{
    (void)(sizeof ... (args));
}

namespace facelift {

FaceliftModelLib_EXPORT Q_DECLARE_LOGGING_CATEGORY(LogGeneral)
FaceliftModelLib_EXPORT Q_DECLARE_LOGGING_CATEGORY(LogModel)

typedef int ModelElementID;

template<typename Type, typename Enable = void>
struct TypeHandler
{
    typedef Type QMLType;
};

template<typename Type>
inline const std::initializer_list<Type> &validValues()
{
    static const std::initializer_list<Type> l;
    return l;
}


template<typename Type>
QString enumToString(const Type &v)
{
    Q_UNUSED(v);
    static_assert(!std::is_enum<Type>::value, "Missing specialization of enumToString() template");
    return "";
}


}
