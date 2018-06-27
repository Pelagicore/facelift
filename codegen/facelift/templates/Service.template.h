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

{% set class = '{0}'.format(interface) %}
{% set comma = joiner(",") %}
{% set hasReadyFlags = interface.hasPropertyWithReadyFlag -%}

/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include <QtCore>
#include "FaceliftModel.h"

// Dependencies
{% for property in interface.properties %}
{{property.type.requiredInclude}}
{% endfor %}

{% for operation in interface.operations %}
{% if operation.hasReturnValue %}
{{operation.type.requiredInclude}}
{% endif %}
{% for parameter in operation.parameters %}
{{parameter.type.requiredInclude}}
{% endfor %}
{% endfor %}

{% for event in interface.signals %}
{% for parameter in event.parameters %}
{{parameter.type.requiredInclude}}
{% endfor %}
{% endfor %}

{{module.namespaceCppOpen}}

class {{class}}QMLFrontend;

{% if hasReadyFlags %}
class {{class}}ReadyFlags
{
    Q_GADGET

public:
{% for property in interface.properties %}
{% if property.tags.hasReadyFlag %}

    Q_PROPERTY(bool {{property.name}} READ {{property.name}})
    bool {{property.name}}()
    {
        return m_{{property.name}} ? *m_{{property.name}} : false;
    }
    bool* m_{{property.name}} = nullptr;
{% endif %}
{% endfor %}
};
{% endif %}

{{interface.comment}}
class {{class}} : public facelift::InterfaceBase {

    Q_OBJECT

public:

    static constexpr const char* FULLY_QUALIFIED_INTERFACE_NAME = "{{interface.qualified_name|lower}}";
    static constexpr const char* INTERFACE_NAME = "{{interface}}";

    static constexpr const int VERSION_MAJOR = {{module.majorVersion}};
    static constexpr const int VERSION_MINOR = {{module.minorVersion}};

    typedef {{class}}QMLFrontend QMLFrontendType;

    friend QMLFrontendType;

    {{class}}(QObject* parent = nullptr);

    static void registerTypes(const char* uri)
    {
        Q_UNUSED( uri);
    }

    {% for property in interface.properties %}
    {% if property.type.is_model -%}
    virtual facelift::Model<{{property.nestedType.fullyQualifiedCppType}}>& {{property.name}}() = 0;

    typedef bool PropertyType_{{property}};   // TODO : use actual type

    facelift::ModelPropertyInterface<{{class}}, {{property.nestedType.fullyQualifiedCppType}}> {{property}}Property()
    {
        return facelift::ModelPropertyInterface<{{class}}, {{property.nestedType.fullyQualifiedCppType}}>();
    }

    {% elif property.type.is_interface -%}

    // Service property
    virtual {{property.interfaceCppType}} {{property}}() = 0;

    typedef bool PropertyType_{{property}};   // TODO : use actual type

    facelift::ServicePropertyInterface<{{class}}, {{property.interfaceCppType}}> {{property}}Property()
    {
        return facelift::ServicePropertyInterface<{{class}}, {{property.interfaceCppType}}>();
    }

    {% if (not property.readonly) %}
    virtual void set{{property}}(const {{property.cppType}}& newValue) = 0;
    {% endif %}

    {% else %}
    virtual const {{property.interfaceCppType}}& {{property}}() const = 0;

    facelift::PropertyInterface<{{class}}, {{property.interfaceCppType}}> {{property}}Property()
    {
        return facelift::PropertyInterface<{{class}}, {{property.interfaceCppType}}>(this, &{{class}}::{{property}}, &{{class}}::{{property}}Changed);
    }

    typedef {{property.interfaceCppType}} PropertyType_{{property}};

    {% if (not property.readonly) %}
    virtual void set{{property}}(const {{property.cppType}}& newValue) = 0;
    {% endif %}

    {% endif %}
    Q_SIGNAL void {{property}}Changed();

    {% endfor %}
    {% for operation in interface.operations %}

    {{operation.comment}}
    virtual {{operation.cppType}} {{operation}}({% set comma = joiner(",") %}
        {% for parameter in operation.parameters %}{{ comma() }}{{parameter.cppType}} {{parameter.name}}{% endfor %}) = 0;
    {% endfor %}
    {% for event in interface.signals %}
    {{event.comment}}
    Q_SIGNAL void {{event}}({% set comma = joiner(",") -%}
        {% for parameter in event.parameters -%}{{ comma() }}{{parameter.cppType}} {{parameter.name}}{% endfor %});
    {% endfor %}


    //private:  TODO : make this field private
    mutable QMLFrontendType* m_qmlFrontend = nullptr;


    {% if hasReadyFlags %}
    const {{class}}ReadyFlags &readyFlags() const
    {
        return m_readyFlags;
    }
    Q_SIGNAL void readyFlagsChanged();

protected:
    {{class}}ReadyFlags m_readyFlags;
    {% endif %}
};


{{module.namespaceCppClose}}

