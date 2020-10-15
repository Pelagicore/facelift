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

#include "IPCTypeHandler.h"
#include <tuple>

namespace facelift {

class DBusSignatureHelper {

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
