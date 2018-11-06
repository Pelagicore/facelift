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

#include <QtCore>
#include <QQmlEngine>

#include "{{interfaceName}}QMLFrontend.h"

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

{{className}}::{{className}}(QObject* parent) : facelift::QMLFrontendBase(parent)
{
}

{{className}}::{{className}}(QQmlEngine* engine) : facelift::QMLFrontendBase(engine)
{
}

void {{className}}::connectProvider({{interfaceName}}& provider)
{
    facelift::QMLFrontendBase::connectProvider(provider);
    m_provider = &provider;
    {% for property in interface.properties %}
    {% if property.type.is_model %}
    connect(m_provider, &ProviderInterfaceType::{{property}}Changed, this, [this]() {
        m_{{property}}Model.setModelProperty(m_provider->{{property}}());
    });
    m_{{property}}Model.setModelProperty(m_provider->{{property}}());
    {% endif %}
    connect(m_provider, &ProviderInterfaceType::{{property.name}}Changed, this, &{{className}}::{{property.name}}Changed);
    {% endfor %}

    {% for event in interface.signals %}
    {% if event.parameters|hasQMLIncompatibleParameter %}
    connect(m_provider, &ProviderInterfaceType::{{event.name}}, this, [this] (
        {%- set comma = joiner(", ") -%}
        {%- for parameter in event.parameters -%}
            {{ comma() }}{{parameter.interfaceCppType}} {{parameter.name}}
        {%- endfor -%}) {
        emit ThisType::{{event.name}}(
        {%- set comma2 = joiner(", ") -%}
        {%- for parameter in event.parameters -%}
            {{comma2()}} facelift::toQMLCompatibleType({{parameter.name}})
        {%- endfor -%});
    });
    {% else %}
    connect(m_provider, &ProviderInterfaceType::{{event.name}}, this, &{{className}}::{{event.name}});
    {% endif %}
    {% endfor %}
    {% if hasReadyFlags %}
    connect(m_provider, &ProviderInterfaceType::readyFlagsChanged, this, &{{className}}::readyFlagsChanged);
    {% endif %}
}


{{module.namespaceCppClose}}
