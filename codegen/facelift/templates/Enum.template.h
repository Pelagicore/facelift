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

#include <QtCore>

#include "FaceliftModel.h"

{{module|namespaceOpen}}

class {{enum}}Gadget
{
    Q_GADGET
public:
    enum Type {
        {% set comma = joiner(",") %}
        {%- for member in enum.members -%}
        {{ comma() }}
        {{member.name}} = {{member.value}}
        {%- endfor %}
    };
    Q_ENUM(Type)

};

typedef {{enum}}Gadget::Type {{enum}};

{{module|namespaceClose}}

Q_DECLARE_METATYPE({{enum|fullyQualifiedCppName}}Gadget::Type)

/*
template <> inline QVariant toVariant(const {{enum|fullyQualifiedCppName}}& v) {
    return static_cast<int>(v);
}
*/


namespace facelift {

template<> inline const QList<{{enum|fullyQualifiedCppName}}>& validValues<{{enum|fullyQualifiedCppName}}>() {
	static QList<{{enum|fullyQualifiedCppName}}> values = {
	{% for member in enum.members %}
	{{enum|fullyQualifiedCppName}}::{{member}},
	{% endfor %}
	};
	return values;
}

template <> inline QString enumToString(const {{enum|fullyQualifiedCppName}}& v) {
    const char* s = "Invalid";
    switch(v) {
    {% for member in enum.members %}
    case {{enum|fullyQualifiedCppName}}::{{member}}:
        s = "{{member}}";
        break;
    {% endfor %}
        default:
            break;
    }
    return s;
}

}

inline void assignFromString(const QString &s, {{enum|fullyQualifiedCppName}}& v)
{
    {% for member in enum.members %}
	if (s == "{{member}}")
		v = {{enum|fullyQualifiedCppName}}::{{member}};
    else
    {% endfor %}
	qFatal("No enum value matching string");
}


inline QTextStream &operator <<(QTextStream &outStream, const {{enum|fullyQualifiedCppName}}& f) {
    outStream << facelift::toString(f);
    return outStream;
}
