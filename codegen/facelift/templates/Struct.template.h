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
{% set verified = struct.verifyStruct %}
{% set comma = joiner(", ") %}

#pragma once

{{classExportDefines}}

#include <QDBusArgument>
#include <QtDBus>
#include "Structure.h"
#include "FaceliftQMLUtils.h"

// Dependencies
{% for field in struct.fields %}
{{field.type.requiredInclude}}
{% endfor %}

{{module.namespaceCppOpen}}

{% if struct.isQObjectWrapperEnabled %}
class {{struct.name}}QObjectWrapper;
{% endif %}

/**
* \ingroup {{struct.module.name|toValidId}}
*/

{{struct.comment}}
class {{classExport}} {{struct.name}} : public facelift::Structure<
    {%- for field in struct.fields -%}
        {{ comma() }}{{field.cppType}}
    {%- endfor -%} >
{
    Q_GADGET

public:

    {% if struct.isQObjectWrapperEnabled %}
    using QObjectWrapperType = {{struct.name}}QObjectWrapper;
    static constexpr bool IsStructWithQObjectWrapperType = true;
    {% else %}
    static constexpr bool IsStructWithoutQObjectWrapperType = true;
    {% endif %}

    static const FieldNames FIELD_NAMES;

    static const QString CLASS_ID;

    {{struct.name}}();

    {{struct.name}}(const {{struct.name}} &other);

    {{struct.name}}& operator=(const {{struct.name}} &right);

    friend QDBusArgument &operator<<(QDBusArgument &argument, const {{struct.name}} &{{struct.name|lower}});
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, {{struct.name}} &{{struct.name|lower}});

    static void registerDBusTypes()
    {
        {% for field in struct.fields %}
        {% if field.type.is_struct %}
        {{field.type.cppType}}::registerDBusTypes();
        {% endif %}
        {% if (not field.type.is_primitive and not field.type.is_model and not field.type.is_interface and not field.type.is_list and not field.type.is_map) %}
        qDBusRegisterMetaType<{{field.cppType}}>();
        {% endif %}
        {% if (field.type.is_list or field.type.is_map) %}
        {% if (not field.type.nested.is_primitive) %}
        qDBusRegisterMetaType<{{field.cppType}}>();
        qDBusRegisterMetaType<{{field.type.nested.cppType}}>();
        {% endif %}
        {% endif %}
        {% endfor %}
    }

    Q_INVOKABLE {{struct.fullyQualifiedCppType}} clone() const;

    {% if struct.isSerializable %}

    Q_PROPERTY(QByteArray serialized READ serialize WRITE deserialize)

    QByteArray serialize() const;

    void deserialize(const QByteArray &array);

    {% endif %}

    QString toString() const;

{% for field in struct.fields %}

    {% if field.comment %}
    {{field.comment}}
    {% endif %}
    Q_PROPERTY({{field.type.qmlCompatibleType}} {{field}} READ qmlCompatible{{field}} WRITE qmlCompatibleSet{{field}})
    {{field.type.qmlCompatibleType}} qmlCompatible{{field.name}}() const;

    const {{field.cppType}} &{{field.name}}() const
    {
        return m_{{field.name}};
    }
    void qmlCompatibleSet{{field.name}}({{field.type.qmlCompatibleType}} value);

    {{struct.name}}& set{{field.name}}({{field.cppType}} value)
    {
        // qDebug() << "Setting field {{field.name}} with value:" << value;
        m_{{field.name}} = value;
        return *this;
    }

{% endfor %}
private:
{% for field in struct.fields %}
    {{field.cppType}}& m_{{field}};
{% endfor -%}
};

inline QDBusArgument &operator<<(QDBusArgument &argument, const {{struct.name}} &{{struct.name|lower}})
{
    argument.beginStructure();
    {% for field in struct.fields %}
    argument << {{struct.name|lower}}.m_{{field}};
    {% endfor -%}
    argument.endStructure();
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, {{struct.name}} &{{struct.name|lower}})
{
    argument.beginStructure();
    {% for field in struct.fields %}
    argument >> {{struct.name|lower}}.m_{{field}};
    {% endfor -%}
    argument.endStructure();

    return argument;
}

{{module.namespaceCppClose}}

inline QTextStream &operator <<(QTextStream &outStream, const {{struct.fullyQualifiedCppType}} &f)
{
    outStream << f.toString();
    return outStream;
}

inline QDebug operator<< (QDebug d, const {{struct.fullyQualifiedCppType}} &f)
{
    QString s;
    QTextStream stream(&s);
    stream << f;
    d << s;
    return d;
}

typedef QMap<QString,{{struct.fullyQualifiedCppType}}> QMapOf{{struct.fullyQualifiedCppType|replace("::","")}};

Q_DECLARE_METATYPE(QList<{{struct.fullyQualifiedCppType}}>)   // Needed for list properties
Q_DECLARE_METATYPE(QMapOf{{struct.fullyQualifiedCppType|replace("::","")}})
Q_DECLARE_METATYPE({{struct.fullyQualifiedCppType}})



