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
{% set comma = joiner(", ") %}

#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <QTextStream>

#include "FaceliftModel.h"
#include "QMLModel.h"

// Dependencies
{% for field in struct.fields %}
{{field|requiredInclude}}
{% endfor %}

{{module|namespaceOpen}}

class {{struct.name}}QObjectWrapper;

/**
* \ingroup {{struct.module.name|toValidId}}
*/

{{struct.comment}}
class {{struct.name}} : public facelift::Structure<
    {%- for field in struct.fields -%}
        {{ comma() }}{{field|returnType}}
    {%- endfor -%} >
{
    Q_GADGET

public:

    typedef {{struct.name}}QObjectWrapper QObjectWrapperType;

    static const FieldNames FIELD_NAMES;

    static const QString &classID()
    {
        static auto id = QStringLiteral("{{struct|fullyQualifiedName}}");
        return id;
    }

    // This seems to be necessary even if the base class already contains an "id" property. TODO: clarify
    Q_PROPERTY(int id READ id WRITE setId)
    Q_PROPERTY(QByteArray serialized READ serialize WRITE deserialize)

    {{struct.name}}()
    {% if struct.fields %}        : {% endif -%}
    {% for field in struct.fields -%}
        m_{{field}}(std::get<{{loop.index-1}}>(m_values))
        {% if not loop.last %}        , {% endif %}
    {% endfor %}
    {
    }

    {{struct.name}}(const {{struct.name}} &other)
    {% if struct.fields %}        : {% endif -%}
    {% for field in struct.fields -%}
        m_{{field}}(std::get<{{loop.index-1}}>(m_values))
        {% if not loop.last %}        , {% endif %}
    {% endfor %}
    {
        copyFrom(other);
    }

    {{struct.name}}& operator=(const {{struct.name}} &right)
    {
        copyFrom(right);
        return *this;
    }

    Q_INVOKABLE {{struct|fullyQualifiedCppName}} clone() const
    {
        {{struct.name}} s;
        s.setValue(asTuple());
        return s;
    }

    QString toString() const
    {
        return toStringWithFields(classID(), FIELD_NAMES);
    }

{% for field in struct.fields %}
    {{field.comment}}
    Q_PROPERTY({{field|qmlCompatibleType}} {{field}} READ qmlCompatible{{field}} WRITE qmlCompatibleSet{{field}})

    {{field|qmlCompatibleType}} qmlCompatible{{field.name}}() const
    {
        return facelift::toQMLCompatibleType({{field.name}}());
    }

    const {{field|returnType}} &{{field.name}}() const
    {
        return m_{{field.name}};
    }

    void qmlCompatibleSet{{field.name}}({{field|qmlCompatibleType}} value)
    {
        // qDebug() << "Setting field {{field.name}} with value:" << value;
        facelift::assignFromQmlType(m_{{field.name}}, value);
    }

    void set{{field.name}}({{field|returnType}} value)
    {
        // qDebug() << "Setting field {{field.name}} with value:" << value;
        m_{{field.name}} = value;
    }
{% endfor %}

private:
{% for field in struct.fields %}
    {{field|returnType}}& m_{{field}};
{% endfor -%}
};


{{module|namespaceClose}}

inline QTextStream &operator <<(QTextStream &outStream, const {{struct|fullyQualifiedCppName}} &f)
{
    outStream << f.toString();
    return outStream;
}

inline QDebug operator<< (QDebug d, const {{struct|fullyQualifiedCppName}} &f)
{
    QString s;
    QTextStream stream(&s);
    stream << f;
    d << s;
    return d;
}

Q_DECLARE_METATYPE(QList<{{struct|fullyQualifiedCppName}}>)   // Needed for list properties
//Q_DECLARE_METATYPE(QMap<QString, {{struct|fullyQualifiedCppName}}>)   // TODO: Needed for map properties?
Q_DECLARE_METATYPE({{struct|fullyQualifiedCppName}})

{{module|namespaceOpen}}

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
        return {{struct|fullyQualifiedCppName}}::classID();
    }

    {{struct.name}}QObjectWrapper(QObject* parent = nullptr) : StructQObjectWrapper(parent)
    {
        init();
    }

    {{struct.name}}QObjectWrapper(const {{struct.name}}& value, QObject* parent = nullptr) : StructQObjectWrapper(parent)
    {
        assignFromGadget(value);
        init();
    }

    void init()
    {
        {% for field in struct.fields %}
        m_{{field.name}}.init(this, &{{struct.name}}QObjectWrapper::{{field.name}}Changed, "{{field.name}}");
        QObject::connect(this, &{{struct.name}}QObjectWrapper::{{field.name}}Changed, this, &{{struct.name}}QObjectWrapper::anyFieldChanged);
        {% endfor %}
    }

{% for field in struct.fields %}
    {{field.comment}}
    Q_PROPERTY({{field|qmlCompatibleType}} {{field}} READ qmlCompatible{{field}} WRITE qmlCompatibleSet{{field}} NOTIFY {{field}}Changed)

    {{field|qmlCompatibleType}} qmlCompatible{{field.name}}() const
    {
        return facelift::toQMLCompatibleType({{field.name}}());
    }

    void qmlCompatibleSet{{field.name}}({{field|qmlCompatibleType}} value)
    {
        assignFromQmlType(m_{{field.name}}, value);
    }

    Q_SIGNAL void {{field}}Changed();

    const {{field|returnType}}& {{field.name}}() const
    {
        return m_{{field.name}}.value();
    }

    void set{{field.name}}({{field|returnType}} value)
    {
        m_{{field.name}} = value;
    }

    facelift::Property<{{field|returnType}}> m_{{field.name}};

{% endfor %}

    /**
     * This property gives you access to the underlying gadget object
     */
    Q_PROPERTY({{struct | fullyQualifiedCppName}} gadget READ gadget)

    {{struct.name}} gadget() const
    {
        {{struct.name}} s;
        {% for field in struct.fields %}
        s.set{{field.name}}(m_{{field.name}}.value());
        {% endfor %}
        s.setId(id());
        return s;
    }

    void assignFromGadget(const {{struct | fullyQualifiedCppName}} &gadget)
    {
        {% for field in struct.fields %}
        m_{{field.name}} = gadget.{{field.name}}();
        {% endfor %}
        m_id = gadget.id();
    }

    /**
     * This property contains the serialized form of the structure
     */
    Q_PROPERTY(QByteArray serialized READ serialized WRITE setSerialized NOTIFY anyFieldChanged)

    QByteArray serialized() const
    {
        return gadget().serialize();
    }

    void setSerialized(const QByteArray &array)
    {
        {{struct | fullyQualifiedCppName}} v;
        v.deserialize(array);
        assignFromGadget(v);
    }

    /**
     * This signal is triggered when one of the fields is changed
     */
    Q_SIGNAL void anyFieldChanged();
};


class QMLImplListProperty{{struct}} : public facelift::TQMLImplListProperty<{{struct | fullyQualifiedCppName}}>
{
    typedef facelift::TQMLImplListProperty<{{struct | fullyQualifiedCppName}}> Base;
};

class QMLImplMapProperty{{struct}} : public facelift::TQMLImplMapProperty<{{struct | fullyQualifiedCppName}}>
{
    typedef facelift::TQMLImplMapProperty<{{struct | fullyQualifiedCppName}}> Base;
};


class {{struct}}Factory : public facelift::StructureFactoryBase {

    Q_OBJECT

public:

    {{struct}}Factory(QQmlEngine* qmlEngine) : facelift::StructureFactoryBase(qmlEngine)
    {
    }

    Q_INVOKABLE {{struct|fullyQualifiedCppName}} create()
    {
        return {{struct}}();
    }
};

{{module|namespaceClose}}

namespace facelift {

template<>
class QMLImplListProperty<{{struct | fullyQualifiedCppName}}> : public {{module|fullyQualifiedCppName}}::QMLImplListProperty{{struct}}
{
};

template<>
class QMLImplMapProperty<{{struct | fullyQualifiedCppName}}> : public {{module|fullyQualifiedCppName}}::QMLImplMapProperty{{struct}}
{
};

}
