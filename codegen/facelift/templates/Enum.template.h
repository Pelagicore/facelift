{#*********************************************************************
**
** Copyright (C) 2018 Luxoft Sweden AB
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
*********************************************************************#}

/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include "FaceliftEnum.h"

{{module.namespaceCppOpen}}

class {{enum}}Gadget
{
    Q_GADGET

public:
    enum Type {
        {%- set comma = joiner(",") -%}
        {%- for member in enum.members -%}
        {{ comma() }}
        {{member.name}} = {{member.value}}
        {%- endfor %}

    };
    Q_ENUM(Type)
};

using {{enum}} = {{enum}}Gadget::Type;

{{module.namespaceCppClose}}

Q_DECLARE_METATYPE({{enum.fullyQualifiedCppType}}Gadget::Type)


namespace facelift {

template<> inline const std::initializer_list<{{enum.fullyQualifiedCppType}}>& validValues<{{enum.fullyQualifiedCppType}}>()
{
    static const std::initializer_list<{{enum.fullyQualifiedCppType}}> values = {
    {% for member in enum.members %}
        {{enum.fullyQualifiedCppType}}::{{member}},
    {% endfor %}
    };
    return values;
}

template <> inline QString enumToString(const {{enum.fullyQualifiedCppType}}& v)
{
    const char* s = "Invalid";
    switch(v) {
    {% for member in enum.members %}
    case {{enum.fullyQualifiedCppType}}::{{member}}:
        s = "{{member}}";
        break;
    {% endfor %}
    default:
        break;
    }
    return s;
}

}


inline void assignFromString(const QString &s, {{enum.fullyQualifiedCppType}}& v)
{
    {% for member in enum.members %}
    if (s == "{{member}}")
        v = {{enum.fullyQualifiedCppType}}::{{member}};
    else
    {% endfor %}
        ::facelift::onAssignFromStringError(s);
}


inline QTextStream &operator <<(QTextStream &outStream, const {{enum.fullyQualifiedCppType}}& f)
{
    outStream << facelift::enumToString(f);
    return outStream;
}
