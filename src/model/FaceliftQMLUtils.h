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

#include "FaceliftCommon.h"
#include "FaceliftConversion.h"
#include "QMLModelTypeHandler.h"

namespace facelift {



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
                qCCritical(LogModel).noquote() << "Error executing JS callback. Error type:" << returnValue.property("name").toString()
                                      << "\nFile:" << returnValue.property("fileName").toString()
                                      << "\nLine:" << returnValue.property("lineNumber").toInt()
                                      << "\nMessage:" << returnValue.property("message").toString()
                                      << "\nStack trace:" << returnValue.property("stack").toString();
            }
        } else {
            qCCritical(LogModel, "Provided JS object is not callable");
        }
    }
}


}
