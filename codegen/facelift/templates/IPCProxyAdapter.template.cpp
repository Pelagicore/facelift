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

{% set className = interfaceName + proxyTypeNameSuffix %}

#include "{{className}}.h"

{{module.namespaceCppOpen}}

{{className}}::{{className}}(QObject *parent) : BaseType(parent)
    {% for property in interface.properties %}
    {% if property.type.is_interface %}
    , m_{{property.name}}Proxy(*this)
    {% endif %}
    {% if property.type.is_model %}
    , m_{{property.name}}(*this)
    {% endif %}
    {% endfor %}
{
    ipc()->setObjectPath(SINGLETON_OBJECT_PATH);
    {% if generateAsyncProxy %}
    ipc()->setSynchronous(false);
    {% endif %}
}

void {{className}}::deserializePropertyValues(InputIPCMessage &msg, bool isCompleteSnapshot)
{
    {% for property in interface.properties %}
    {% if property.type.is_interface %}
    QString {{property.name}}_objectPath;
    if (deserializeOptionalValue(msg, {{property.name}}_objectPath, isCompleteSnapshot))
    {
        m_{{property.name}}Proxy.update({{property.name}}_objectPath);
        m_{{property.name}} = m_{{property.name}}Proxy.getValue();
    }
    {% elif property.type.is_model %}
    if (isCompleteSnapshot) {
        int {{property.name}}Size;
        deserializeValue(msg, {{property.name}}Size);
        m_{{property.name}}.beginResetModel();
        m_{{property.name}}.reset({{property.name}}Size, std::bind(&ThisType::{{property.name}}Data, this, std::placeholders::_1));
        m_{{property.name}}.endResetModel();
    }
    {% else %}
    deserializeOptionalValue(msg, m_{{property.name}}, isCompleteSnapshot);
    {% endif %}
    {% endfor %}
    BaseType::deserializePropertyValues(msg, isCompleteSnapshot);
}

void {{className}}::emitChangeSignals() {
{% for property in interface.properties %}
    emit {{property.name}}Changed();
{% endfor %}
    BaseType::emitChangeSignals();
}

void {{className}}::deserializeSignal(InputIPCMessage &msg)
{
    SignalID member;
    deserializeValue(msg, member);

    {% for event in interface.signals %}
    if (member == SignalID::{{event}})
    {
        {% for parameter in event.parameters %}
        {{parameter.interfaceCppType}} param_{{parameter.name}};
        deserializeValue(msg, param_{{parameter.name}});
        {% endfor %}
        emit {{event}}(
        {%- set comma = joiner(", ") -%}
        {%- for parameter in event.parameters -%}
            {{ comma() }}param_{{parameter.name}}
        {%- endfor -%}  );
    } else
    {% endfor %}
    {% for property in interface.properties %}
    if (member == SignalID::{{property.name}}) {
    {% if property.type.is_model %}
        m_{{property.name}}.handleSignal(msg);
    {% else %}
        emit {{property.name}}Changed();
    {% endif %}
    } else
    {% endfor %}
        BaseType::deserializeCommonSignal(static_cast<facelift::CommonSignalID>(member), this);
}

{% for property in interface.properties %}

{% if (not property.readonly) %}

void {{className}}::set{{property}}({{property.cppMethodArgumentType}} newValue)
{
    {% if (not property.type.is_interface) %}
    ipc()->sendSetterCall(memberID(MethodID::set{{property.name}}, "set{{property.name}}"), newValue);
    {% else %}
    Q_ASSERT(false); // Writable interface properties are unsupported
    {% endif %}
}
{% endif %}

{% if property.type.is_model %}
{{property.nestedType.interfaceCppType}} {{className}}::{{property.name}}Data(int row)
{
    return m_{{property.name}}.modelData(memberID(MethodID::{{property.name}}, "{{property.name}}"), row);
}
{% endif %}
{% endfor %}


{% for operation in interface.operations %}

{% if operation.isAsync %}
void {{className}}::{{operation.name}}(
    {%- for parameter in operation.parameters -%}{{parameter.cppMethodArgumentType}} {{parameter.name}}, {% endfor %}facelift::AsyncAnswer<{{operation.interfaceCppType}}> answer){% if operation.is_const %} const{% endif %} {
        ipc()->sendAsyncMethodCall(memberID(MethodID::{{operation.name}}, "{{operation.name}}"), answer
        {%- for parameter in operation.parameters -%}
        , {{parameter.name}}
        {%- endfor -%}  );
}
{% else %}
{{operation.interfaceCppType}} {{className}}::{{operation.name}}(
    {%- set comma = joiner(", ") -%}
    {%- for parameter in operation.parameters -%}
    {{ comma() }}{{ parameter.cppMethodArgumentType }} {{parameter.name}}
    {%- endfor -%}  ){% if operation.is_const %} const{% endif %}
{
        {% if (operation.hasReturnValue) %}
        {{operation.interfaceCppType}} returnValue;
        ipc()->sendMethodCallWithReturn(memberID(MethodID::{{operation.name}}, "{{operation.name}}"), returnValue
            {%- for parameter in operation.parameters -%}
            , {{parameter.name}}
            {%- endfor -%} );
        return returnValue;
        {% else %}
        ipc()->sendMethodCall(memberID(MethodID::{{operation.name}}, "{{operation.name}}")
        {%- for parameter in operation.parameters -%}
        , {{parameter.name}}
        {%- endfor -%}  );
        {% endif %}
}
{% endif %}
{% endfor %}


{{module.namespaceCppClose}}
