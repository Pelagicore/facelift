{#*********************************************************************
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
*********************************************************************#}

{% set ampersand = joiner(" &&") %}
{% set comma = joiner(", ") %}
/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#include "{{struct}}QObjectWrapper.h"
#include "FaceliftConversion.h"

{{module.namespaceCppOpen}}


void {{struct.name}}QObjectWrapper::init()
{
    {% for field in struct.fields %}
    m_{{field.name}}.init(this, &{{struct.name}}QObjectWrapper::{{field.name}}Changed, "{{field.name}}");
    QObject::connect(this, &{{struct.name}}QObjectWrapper::{{field.name}}Changed, this, &{{struct.name}}QObjectWrapper::anyFieldChanged);
    {% endfor %}
    {% if struct.isQObjectWrapperDeprecated %}
    qCritical() << "QFace definition of structure '{{struct.qualified_name}}' is missing '@qml-component: true' annotation. This type will not be created anymore in the next version of Facelift.";
    {% endif %}
}


{{struct.name}} {{struct.name}}QObjectWrapper::gadget() const
{
    {{struct.name}} s;
    {% for field in struct.fields %}
    s.set{{field.name}}(m_{{field.name}}.value());
    {% endfor %}
    s.setId(id());
    return s;
}

void {{struct.name}}QObjectWrapper::assignFromGadget(const {{struct.fullyQualifiedCppType}} &gadget)
{
    {% for field in struct.fields %}
    m_{{field.name}} = gadget.{{field.name}}();
    {% endfor %}
    m_id = gadget.id();
}

{% if struct.isSerializable %}

QByteArray {{struct.name}}QObjectWrapper::serialized() const
{
    return facelift::serializeStructure(gadget());
}

void {{struct.name}}QObjectWrapper::setSerialized(const QByteArray &array)
{
    {{struct.fullyQualifiedCppType}} v;
    facelift::deserializeStructure(v, array);
    assignFromGadget(v);
}

{% endif %}

{{struct.name}}QObjectWrapper::{{struct.name}}QObjectWrapper(QObject* parent) : StructQObjectWrapper(parent)
{
    init();
}

{{struct.name}}QObjectWrapper::{{struct.name}}QObjectWrapper(const {{struct.name}}& value, QObject* parent) : StructQObjectWrapper(parent)
{
    assignFromGadget(value);
    init();
}

{% for field in struct.fields %}

    {{field.type.qmlCompatibleType}} {{struct.name}}QObjectWrapper::qmlCompatible{{field.name}}() const
    {
        return facelift::toQMLCompatibleType({{field.name}}());
    }
    void {{struct.name}}QObjectWrapper::qmlCompatibleSet{{field.name}}({{field.type.qmlCompatibleType}} value)
    {
        assignFromQmlType(m_{{field.name}}, value);
    }
    const {{field.cppType}}& {{struct.name}}QObjectWrapper::{{field.name}}() const
    {
        return m_{{field.name}}.value();
    }
    void {{struct.name}}QObjectWrapper::set{{field.name}}({{field.cppType}} value)
    {
        m_{{field.name}} = value;
    }

{% endfor %}

{{module.namespaceCppClose}}
