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

/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/
{% set verified = struct.verifyStruct %}
{% set comma = joiner(", ") %}

#pragma once

#include "{{struct.name}}.h"
#include "StructQObjectWrapper.h"

// Dependencies
{% for field in struct.fields %}
{% if field.type.requiredInclude != "" %}
{{field.type.requiredInclude}}
{% endif %}
{% endfor %}

{{module.namespaceCppOpen}}

{{struct.comment}}

/**
* \class {{struct.name}}QObjectWrapper
* \ingroup {{struct.module.name|toValidId}}
* \inqmlmodule {{struct.module.name}}
*/
class {{struct.name}}QObjectWrapper : public facelift::StructQObjectWrapper<{{struct.name}}>
{
    Q_OBJECT

public:

    Q_PROPERTY(QString classID READ classID CONSTANT)

    static const QString& classID()
    {
        return {{struct.fullyQualifiedCppType}}::CLASS_ID;
    }

{% if struct.isQObjectWrapperDeprecated %}
    [[deprecated("QFace definition of structure '{{struct.qualified_name}}' is missing '@qml-component: true' annotation. This type will not be created anymore in the next version of Facelift.")]]
{% endif %}
    {{struct.name}}QObjectWrapper(QObject* parent = nullptr);

{% if struct.isQObjectWrapperDeprecated %}
    [[deprecated("QFace definition of structure '{{struct.qualified_name}}' is missing '@qml-component: true' annotation. This type will not be created anymore in the next version of Facelift.")]]
{% endif %}
    {{struct.name}}QObjectWrapper(const {{struct.name}}& value, QObject* parent = nullptr);

    void init();

{% for field in struct.fields %}

    {% if field.comment %}
    {{field.comment}}
    {% endif %}
public:
    Q_PROPERTY({{field.type.qmlCompatibleType}} {{field}} READ qmlCompatible{{field}} WRITE qmlCompatibleSet{{field}} NOTIFY {{field}}Changed)

    {{field.type.qmlCompatibleType}} qmlCompatible{{field.name}}() const;

    void qmlCompatibleSet{{field.name}}({{field.type.qmlCompatibleType}} value);

    const {{field.cppType}}& {{field.name}}() const;

    void set{{field.name}}({{field.cppType}} value);

    Q_SIGNAL void {{field}}Changed();

private:
    facelift::Property<{{field.cppType}}> m_{{field.name}};

{% endfor %}

public:
    /**
     * This property gives you access to the underlying gadget object
     */
    Q_PROPERTY({{struct.fullyQualifiedCppType}} gadget READ gadget)

    {{struct.name}} gadget() const;

    void assignFromGadget(const {{struct.fullyQualifiedCppType}} &gadget);


    {% if struct.isSerializable %}

    /**
     * This property contains the serialized form of the structure
     */
    Q_PROPERTY(QByteArray serialized READ serialized WRITE setSerialized NOTIFY anyFieldChanged)

    QByteArray serialized() const;

    void setSerialized(const QByteArray &array);

    {% endif %}

};

{{module.namespaceCppClose}}

{{module.namespaceCppOpen}}


class QMLImplListProperty{{struct}} : public facelift::TQMLImplListProperty<{{struct.fullyQualifiedCppType}}>
{
    using Base = facelift::TQMLImplListProperty<{{struct.fullyQualifiedCppType}}>;
};

class QMLImplMapProperty{{struct}} : public facelift::TQMLImplMapProperty<{{struct.fullyQualifiedCppType}}>
{
    using Base = facelift::TQMLImplMapProperty<{{struct.fullyQualifiedCppType}}>;
};

{{module.namespaceCppClose}}


namespace facelift {

template<>
class QMLImplListProperty<{{struct.fullyQualifiedCppType}}> : public {{module.fullyQualifiedCppType}}::QMLImplListProperty{{struct}}
{
};

template<>
class QMLImplMapProperty<{{struct.fullyQualifiedCppType}}> : public {{module.fullyQualifiedCppType}}::QMLImplMapProperty{{struct}}
{
};

}
