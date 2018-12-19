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

#include <QTextStream>

#include "StructureBase.h"
#include "QMLModel.h"

// Dependencies
{% for field in struct.fields %}
{{field.type.requiredInclude}}
{% endfor %}

{{module.namespaceCppOpen}}

class {{struct.name}}QObjectWrapper;

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

    using QObjectWrapperType = {{struct.name}}QObjectWrapper;

    static const FieldNames FIELD_NAMES;

    static const QString CLASS_ID;

    // This seems to be necessary even if the base class already contains an "id" property. TODO: clarify
    Q_PROPERTY(int id READ id WRITE setId)
    Q_PROPERTY(QByteArray serialized READ serialize WRITE deserialize)

    {{struct.name}}();

    {{struct.name}}(const {{struct.name}} &other);

    {{struct.name}}& operator=(const {{struct.name}} &right);

    Q_INVOKABLE {{struct.fullyQualifiedCppType}} clone() const;

    QString toString() const;

{% for field in struct.fields %}

    {% if field.comment %}
    {{field.comment}}
    {% endif %}
    Q_PROPERTY({{field.type.qmlCompatibleType}} {{field}} READ qmlCompatible{{field}} WRITE qmlCompatibleSet{{field}})
    {{field.type.qmlCompatibleType}} qmlCompatible{{field.name}}() const
    {
        return facelift::toQMLCompatibleType({{field.name}}());
    }
    const {{field.cppType}} &{{field.name}}() const
    {
        return m_{{field.name}};
    }
    void qmlCompatibleSet{{field.name}}({{field.type.qmlCompatibleType}} value)
    {
        // qDebug() << "Setting field {{field.name}} with value:" << value;
        facelift::assignFromQmlType(m_{{field.name}}, value);
    }
    void set{{field.name}}({{field.cppType}} value)
    {
        // qDebug() << "Setting field {{field.name}} with value:" << value;
        m_{{field.name}} = value;
    }

{% endfor %}
private:
{% for field in struct.fields %}
    {{field.cppType}}& m_{{field}};
{% endfor -%}
};


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

Q_DECLARE_METATYPE(QList<{{struct.fullyQualifiedCppType}}>)   // Needed for list properties
//Q_DECLARE_METATYPE(QMap<QString, {{struct.fullyQualifiedCppType}}>)   // TODO: Needed for map properties?
Q_DECLARE_METATYPE({{struct.fullyQualifiedCppType}})

{{module.namespaceCppOpen}}

{{struct.comment}}

/**
* \class {{struct.name}}QObjectWrapper
* \ingroup {{struct.module.name|toValidId}}
* \inqmlmodule {{struct.module.name}}
*/
class {{classExport}} {{struct.name}}QObjectWrapper : public facelift::StructQObjectWrapper<{{struct.name}}>
{
    Q_OBJECT

public:

    Q_PROPERTY(QString classID READ classID CONSTANT)

    static const QString& classID()
    {
        return {{struct.fullyQualifiedCppType}}::CLASS_ID;
    }

    {{struct.name}}QObjectWrapper(QObject* parent = nullptr);

    {{struct.name}}QObjectWrapper(const {{struct.name}}& value, QObject* parent = nullptr);

    void init();

{% for field in struct.fields %}

    {% if field.comment %}
    {{field.comment}}
    {% endif %}
    Q_PROPERTY({{field.type.qmlCompatibleType}} {{field}} READ qmlCompatible{{field}} WRITE qmlCompatibleSet{{field}} NOTIFY {{field}}Changed)
    {{field.type.qmlCompatibleType}} qmlCompatible{{field.name}}() const
    {
        return facelift::toQMLCompatibleType({{field.name}}());
    }
    void qmlCompatibleSet{{field.name}}({{field.type.qmlCompatibleType}} value)
    {
        assignFromQmlType(m_{{field.name}}, value);
    }
    Q_SIGNAL void {{field}}Changed();
    const {{field.cppType}}& {{field.name}}() const
    {
        return m_{{field.name}}.value();
    }
    void set{{field.name}}({{field.cppType}} value)
    {
        m_{{field.name}} = value;
    }
    facelift::Property<{{field.cppType}}> m_{{field.name}};

{% endfor %}

    /**
     * This property gives you access to the underlying gadget object
     */
    Q_PROPERTY({{struct.fullyQualifiedCppType}} gadget READ gadget)

    {{struct.name}} gadget() const;

    void assignFromGadget(const {{struct.fullyQualifiedCppType}} &gadget);

    /**
     * This property contains the serialized form of the structure
     */
    Q_PROPERTY(QByteArray serialized READ serialized WRITE setSerialized NOTIFY anyFieldChanged)

    QByteArray serialized() const;

    void setSerialized(const QByteArray &array);

    /**
     * This signal is triggered when one of the fields is changed
     */
    Q_SIGNAL void anyFieldChanged();
};


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
