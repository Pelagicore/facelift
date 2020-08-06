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

#include <QQmlEngine>
#include <QJSValue>

#include "FaceliftCommon.h"
#include "FaceliftConversion.h"

namespace facelift {

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


}
