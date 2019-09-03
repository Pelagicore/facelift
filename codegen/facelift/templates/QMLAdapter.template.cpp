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

{% set className = interfaceName + "QMLAdapter" %}
{% set hasReadyFlags = interface.hasPropertyWithReadyFlag %}
{%- macro printif(name) -%}
{%- if name -%}{{name}}
{% endif -%}
{%- endmacro -%}

/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#include <QtCore>
#include <QQmlEngine>

#include "FaceliftQMLUtils.h"

#include "{{interfaceName}}QMLAdapter.h"

{{module.namespaceCppOpen}}


/**
* \class {{className}}
* \ingroup {{interface.module.name|toValidId}}
* \inqmlmodule {{interface.module.name}}
*/

{{className}}::{{className}}(QObject* parent) : facelift::QMLAdapterBase(parent)
{
}

{{className}}::{{className}}(QQmlEngine* engine) : facelift::QMLAdapterBase(engine)
{
}

void {{className}}::connectProvider({{interfaceName}}& provider)
{
    facelift::QMLAdapterBase::connectProvider(provider);
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


{% for property in interface.properties %}

{{- printif(property.comment)}}
{% if property.type.is_model %}
{% elif property.type.is_list or property.type.is_map %}
{{property.type.qmlCompatibleType}} {{className}}::{{property}}() const
{
    return facelift::toQMLCompatibleType(m_provider->{{property}}());
}
    {% if not property.readonly %}
void {{className}}::set{{property}}(const {{property.type.qmlCompatibleType}}& newValue)
{
    // qDebug() << "Request to set property {{property}} to " << newValue;
    Q_ASSERT(m_provider);
    {{property.cppType}} tmp;
    facelift::assignFromQmlType(tmp, newValue);
    m_provider->set{{property}}(tmp);
}
    {% endif %}
{%- elif property.type.is_interface %}
{{property.cppType}}QMLAdapter* {{className}}::{{property}}()
{
    auto qmlAdapter = facelift::getQMLAdapter(m_provider->{{property}}());
    if (qmlAdapter != nullptr)
        qmlAdapter->setParentQMLAdapter(this);
    return qmlAdapter;
}
{% else %}
    {% if property.readonly %}
    {%- else %}
    {%- endif %}

{% endif %}
{% endfor %}


{% for operation in interface.operations %}

{% if operation.isAsync %}
void {{className}}::{{operation}}(
    {%- set comma = joiner(", ") -%}
    {%- for parameter in operation.parameters -%}
        {{ comma() }}{{parameter.type.qmlCompatibleType}} {{parameter.name}}
    {%- endfor -%}
    {{ comma() }}QJSValue callback){% if operation.is_const %} const{% endif %}
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
        facelift::callJSCallback(qmlEngine(), callback{% if operation.hasReturnValue %}, returnValue{% endif %});
    }));
}
{% else %}
{{operation.type.qmlCompatibleType}} {{className}}::{{operation}}(
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

{{module.namespaceCppClose}}
