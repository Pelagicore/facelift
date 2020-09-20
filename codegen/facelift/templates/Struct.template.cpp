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

{% set ampersand = joiner(" &&") %}
{% set comma = joiner(", ") %}
/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#include "{{struct}}.h"
#include "FaceliftConversion.h"

{{module.namespaceCppOpen}}


const QString {{struct.name}}::CLASS_ID = QStringLiteral("{{struct.qualified_name}}");

{{struct.name}}::{{struct.name}}(){% if struct.fields %} : {% endif %}
{% for field in struct.fields %}

    m_{{field}}(std::get<{{loop.index-1}}>(m_values)){% if not loop.last %}, {% endif %}
{% endfor %}

{
}

{{struct.name}}::{{struct.name}}(const {{struct.name}} &other){% if struct.fields %} : {% endif %}
{% for field in struct.fields %}

    m_{{field}}(std::get<{{loop.index-1}}>(m_values)){% if not loop.last %}, {% endif %}
{% endfor %}

{
    copyFrom(other);
}

{% for field in struct.fields %}

    {{field.type.qmlCompatibleType}} {{struct.name}}::qmlCompatible{{field.name}}() const
    {
        return facelift::toQMLCompatibleType({{field.name}}());
    }
    void {{struct.name}}::qmlCompatibleSet{{field.name}}({{field.type.qmlCompatibleType}} value)
    {
        // qDebug() << "Setting field {{field.name}} with value:" << value;
        facelift::assignFromQmlType(m_{{field.name}}, value);
    }

{% endfor %}


{% if struct.isSerializable %}

QByteArray {{struct.name}}::serialize() const
{
    QByteArray array;
    facelift::BinarySeralizer ds(array);
    ds << *this;
    return array;
}

void {{struct.name}}::deserialize(const QByteArray &array)
{
    facelift::BinarySeralizer ds(array);
    ds >> *this;
}

{% endif %}

{% if struct.toByteArrayOverDBus %}
bool {{struct.name}}::toByteArrayOverDBus()
{
    return true;
}
{% endif %}

const {{struct}}::FieldNames {{struct}}::FIELD_NAMES = { {
    {%- for field in struct.fields -%}
    {{ comma() }}
    "{{field.name}}"
    {%- endfor %}
} };


{{struct.fullyQualifiedCppType}} {{struct.name}}::clone() const
{
    {{struct.name}} s;
    s.setValue(asTuple());
    return s;
}


{{struct.name}}& {{struct.name}}::operator=(const {{struct.name}} &right)
{
    if (this != &right) {
        copyFrom(right);
    }
    return *this;
}


QString {{struct.name}}::toString() const
{
    return toStringWithFields(CLASS_ID, FIELD_NAMES);
}

{{module.namespaceCppClose}}
