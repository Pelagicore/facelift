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

{% set hasReadyFlags = interface.hasPropertyWithReadyFlag -%}

/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

{{classExportDefines}}

#include "FaceliftModel.h"
#include <QtDBus>

// Dependencies
{% for type in interface.referencedTypes %}
{{type.requiredInclude}}
{% endfor %}

{{module.namespaceCppOpen}}

class {{interfaceName}}QMLAdapter;
class {{interface}}IPCDBusAdapter;
class {{interfaceName}}IPCDBusProxy;
class {{interface}}IPCLocalServiceAdapter;
class {{interfaceName}}IPCLocalProxyAdapter;
class {{interfaceName}}ImplementationBaseQML;

{% if hasReadyFlags %}
class {{classExport}} {{interfaceName}}ReadyFlags
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
class {{classExport}} {{interfaceName}} : public facelift::InterfaceBase
{
    Q_OBJECT

public:
    static constexpr const char* FULLY_QUALIFIED_INTERFACE_NAME = "{{interface.qualified_name|lower}}";
    static constexpr const char* INTERFACE_NAME = "{{interface}}";
    static constexpr const char* SINGLETON_OBJECT_PATH = "/singletons/{{interface.qualified_name|lower|replace(".","/")}}";

    static constexpr const int VERSION_MAJOR = {{module.majorVersion}};
    static constexpr const int VERSION_MINOR = {{module.minorVersion}};

    using QMLAdapterType = {{interfaceName}}QMLAdapter;
    using IPCDBusAdapterType = {{interface}}IPCDBusAdapter;
    using IPCDBusProxyType = {{interfaceName}}IPCDBusProxy;
    using IPCLocalAdapterType = {{interface}}IPCLocalServiceAdapter;
    using IPCLocalProxyType = {{interfaceName}}IPCLocalProxyAdapter;
    using ThisType = {{interfaceName}};
    using ImplementationBaseQMLType = {{interfaceName}}ImplementationBaseQML;
    using InterfaceType = {{interfaceName}};

    friend QMLAdapterType;

    {{interfaceName}}(QObject* parent = nullptr);

    static void registerTypes(const char* uri)
    {
        Q_UNUSED( uri);
    }

    {% for property in interface.properties %}
    {% if property.type.is_model %}
    using PropertyType_{{property}} = bool;   // TODO : use actual type
    virtual facelift::Model<{{property.nestedType.interfaceCppType}}>& {{property.name}}() = 0;
    facelift::ModelPropertyInterface<ThisType, {{property.nestedType.interfaceCppType}}> {{property}}Property()
    {
        return facelift::ModelPropertyInterface<ThisType, {{property.nestedType.interfaceCppType}}>(this, {{property.name}}());
    }
    {% elif property.type.is_interface %}
    // Service property
    using PropertyType_{{property}} = bool;   // TODO : use actual type
    virtual {{property.interfaceCppType}} {{property}}() = 0;
    facelift::ServicePropertyInterface<ThisType, {{property.cppType}}> {{property}}Property()
    {
        return facelift::ServicePropertyInterface<ThisType, {{property.cppType}}>(this, &ThisType::{{property}}, &ThisType::{{property}}Changed);
    }
    {% else %}
    using PropertyType_{{property}} = {{property.interfaceCppType}};
    virtual const {{property.interfaceCppType}}& {{property}}() const = 0;
    facelift::PropertyInterface<ThisType, {{property.interfaceCppType}}> {{property}}Property()
    {
        return facelift::PropertyInterface<ThisType, {{property.interfaceCppType}}>(this, &ThisType::{{property}}, &ThisType::{{property}}Changed, "{{property.name}}");
    }
    {% if (not property.readonly) %}
    virtual void set{{property}}( {{property.cppMethodArgumentType}} newValue) = 0;
    {% endif %}
    {% endif %}
    Q_SIGNAL void {{property}}Changed();

    {% endfor %}

    {% for operation in interface.operations %}
    {% if operation.isAsync %}
    virtual void {{operation}}(
        {%- for parameter in operation.parameters -%} {{parameter.cppMethodArgumentType}} {{parameter.name}}, {% endfor %}facelift::AsyncAnswer<{{operation.interfaceCppType}}> answer = facelift::AsyncAnswer<{{operation.interfaceCppType}}>()){% if operation.is_const %} const{% endif %} = 0;
    {% else %}

    {% if operation.comment %}
    {{operation.comment}}
    {% endif %}
    virtual {{operation.interfaceCppType}} {{operation}}({% set comma = joiner(",") %}
        {% for parameter in operation.parameters %}{{ comma() }}{{parameter.cppMethodArgumentType}} {{parameter.name}}{% endfor %}){% if operation.is_const %} const{% endif %} = 0;
    {% endif %}
    {% endfor %}

    {% for event in interface.signals %}
    {% if event.comment %}
    {{event.comment}}
    {% endif %}
    Q_SIGNAL void {{event}}({% set comma = joiner(",") -%}
        {% for parameter in event.parameters -%}{{ comma() }}{{parameter.cppType}} {{parameter.name}}{% endfor %});
    {% endfor %}

    {% if hasReadyFlags %}
    const {{interfaceName}}ReadyFlags &readyFlags() const
    {
        return m_readyFlags;
    }
    Q_SIGNAL void readyFlagsChanged();

    {% endif %}
    //private:  TODO : make this field private
    mutable QMLAdapterType* m_qmlAdapter = nullptr;
    {% if hasReadyFlags %}

protected:
    {{interfaceName}}ReadyFlags m_readyFlags;
    {% endif %}
};


{{module.namespaceCppClose}}

{% if hasReadyFlags %}
Q_DECLARE_METATYPE({{module.fullyQualifiedCppType}}::{{interfaceName}}ReadyFlags)
{% endif %}
