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

{% set className = interfaceName + "QMLFrontend" %}
{% set hasReadyFlags = interface.hasPropertyWithReadyFlag %}
{%- macro printif(name) -%}
{%- if name -%}{{name}}
{% endif -%}
{%- endmacro -%}

/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

{{classExportDefines}}

#include <QtCore>
#include <QQmlEngine>

#include "QMLFrontend.h"
#include "{{interfaceName}}.h"

// Dependencies
{% for property in interface.properties -%}
{{- printif(property.type.requiredInclude) }}
{{- printif(property.type.requiredQMLInclude) }}
{%- endfor -%}
{% for operation in interface.operations -%}
{% for parameter in operation.parameters -%}
{{- printif(parameter.type.requiredInclude) }}
{{- printif(parameter.type.requiredQMLInclude) }}
{%- endfor %}
{%- endfor %}
{% for event in interface.signals -%}
{% for parameter in event.parameters -%}
{{- printif(parameter.type.requiredInclude) }}
{{- printif(parameter.type.requiredQMLInclude) }}
{%- endfor %}
{%- endfor %}

{{module.namespaceCppOpen}}


/**
* \class {{className}}
* \ingroup {{interface.module.name|toValidId}}
* \inqmlmodule {{interface.module.name}}
*/

/**
 * This is the class registered as a QML component for the {{interfaceName}} interface
 */
class {{classExport}} {{className}} : public facelift::QMLFrontendBase
{
    Q_OBJECT

    using ProviderInterfaceType = {{interfaceName}};
    using ThisType = {{className}};

public:

    {{className}}(QObject* parent);

    {{className}}(QQmlEngine* engine);

    void init({{interfaceName}}& provider);

    {% if hasReadyFlags %}

    Q_PROPERTY({{module.fullyQualifiedCppType}}::{{interfaceName}}ReadyFlags readyFlags READ readyFlags NOTIFY readyFlagsChanged)
    {{interfaceName}}ReadyFlags readyFlags() const
    {
        return m_provider->readyFlags();
    }
    Q_SIGNAL void readyFlagsChanged();
    {% endif %}

    {% for property in interface.properties %}

    {{- printif(property.comment)}}
    {% if property.type.is_model %}
    Q_PROPERTY(QAbstractListModel* {{property}} READ {{property}} NOTIFY {{property.name}}Changed)
    QAbstractListModel* {{property}}()
    {
        return &m_{{property}}Model;
    }
    facelift::ModelListModel<{{property.nestedType.interfaceCppType}}> m_{{property}}Model;
    {% elif property.type.is_list or property.type.is_map %}
    // Using {{property.type.qmlCompatibleType}}, since exposing {{property.interfaceCppType}} to QML does not seem to work
    Q_PROPERTY({{property.type.qmlCompatibleType}} {{property}} READ {{property}}
               {%- if not property.readonly %} WRITE set{{property}}{% endif %} NOTIFY {{property.name}}Changed)
    {{property.type.qmlCompatibleType}} {{property}}() const
    {
        return facelift::toQMLCompatibleType(m_provider->{{property}}());
    }
        {% if not property.readonly %}
    void set{{property}}(const {{property.type.qmlCompatibleType}}& newValue)
    {
        // qDebug() << "Request to set property {{property}} to " << newValue;
        Q_ASSERT(m_provider);
        {{property.cppType}} tmp;
        facelift::assignFromQmlType(tmp, newValue);
        m_provider->set{{property}}(tmp);
    }
        {% endif %}
    {%- elif property.type.is_interface %}
    Q_PROPERTY({{property.cppType}}QMLFrontend* {{property}} READ {{property}} NOTIFY {{property.name}}Changed)
    {{property.cppType}}QMLFrontend* {{property}}()
    {
        return facelift::getQMLFrontend(m_provider->{{property}}());
    }
    {% else %}
        {% if property.readonly %}
    Q_PROPERTY({{property.type.qmlCompatibleType}} {{property}} READ {{property}} NOTIFY {{property.name}}Changed)
        {%- else %}
    Q_PROPERTY({{property.type.qmlCompatibleType}} {{property}} READ {{property}} WRITE set{{property}} NOTIFY {{property.name}}Changed)
    void set{{property}}(const {{property.cppType}}& newValue)
    {
        // qDebug() << "Request to set property {{property}} to " << newValue;
        Q_ASSERT(m_provider);
        m_provider->set{{property}}(newValue);
    }
        {%- endif %}

    const {{property.cppType}}& {{property}}() const
    {
        Q_ASSERT(m_provider);
        // qDebug() << "Read property {{property}}. Value: " << m_provider->{{property}}() ;
        return m_provider->{{property}}();
    }
    {% endif %}
    Q_SIGNAL void {{property}}Changed();
    {% endfor %}

    {% for operation in interface.operations %}

    {% if operation.isAsync %}
    Q_INVOKABLE void {{operation}}(
        {%- set comma = joiner(", ") -%}
        {%- for parameter in operation.parameters -%}
            {{ comma() }}{{parameter.type.qmlCompatibleType}} {{parameter.name}}
        {%- endfor -%}
        {{ comma() }}QJSValue callback = NO_OPERATION_JS_CALLBACK){% if operation.is_const %} const{% endif %}
    {
        Q_ASSERT(m_provider);
        m_provider->{{operation}}(
            {%- for parameter in operation.parameters -%}
            {%- if parameter.cppType == parameter.type.qmlCompatibleType -%}
            {{parameter.name}},
            {%- else -%}
            facelift::toProviderCompatibleType<{{parameter.cppType}}, {{parameter.type.qmlCompatibleType}}>({{parameter.name}}),
            {%- endif -%}
            {%- endfor -%}
            facelift::AsyncAnswer<{{operation.interfaceCppType}}>(this, [this, callback]({% if operation.hasReturnValue %} {{operation.interfaceCppType}} const &returnValue{% endif %}) mutable {
            callJSCallback(callback{% if operation.hasReturnValue %}, returnValue{% endif %});
        }));
    }
    {% else %}
    Q_INVOKABLE {{operation.type.qmlCompatibleType}} {{operation}}(
        {%- set comma = joiner(", ") -%}
        {%- for parameter in operation.parameters -%}
        {{ comma() }}{{parameter.type.qmlCompatibleType}} {{parameter.name}}
        {%- endfor -%}
    )
    {
        Q_ASSERT(m_provider);
        {% if operation.hasReturnValue %}
        return facelift::toQMLCompatibleType(m_provider->{{operation}}(
        {%- else %}
        m_provider->{{operation}}(
        {%- endif -%}
        {%- set comma = joiner(", ") -%}
        {%- for parameter in operation.parameters -%}
        {{ comma() }}
        {%- if parameter.cppType == parameter.type.qmlCompatibleType -%}
        {{parameter.name}}
        {%- else -%}
        facelift::toProviderCompatibleType<{{parameter.cppType}}, {{parameter.type.qmlCompatibleType}}>({{parameter.name}})
        {%- endif -%}
        {%- endfor -%}
        ){% if operation.hasReturnValue %}){% endif %};
    }
    {% endif %}
    {% endfor %}

    {% for event in interface.signals %}
    Q_SIGNAL void {{event}}(
        {%- set comma = joiner(", ") -%}
        {%- for parameter in event.parameters -%}
        {{ comma() }}{{parameter.type.qmlCompatibleType}} {{parameter.name}}
        {%- endfor -%}
    );
    {% endfor %}

    QPointer<ProviderInterfaceType> m_provider;
};


{{module.namespaceCppClose}}
