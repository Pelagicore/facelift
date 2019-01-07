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

{% set className = interfaceName + "IPCDBusProxy" %}

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

void {{className}}::deserializePropertyValues(::facelift::dbus::DBusIPCMessage &msg, bool isCompleteSnapshot)
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

void {{className}}::deserializeSignal(::facelift::dbus::DBusIPCMessage &msg)
{
    SignalID member;
    deserializeValue(msg, member);

    switch (member) {
    {% for event in interface.signals %}
    case SignalID::{{event}}: {
        {% for parameter in event.parameters %}
        {{parameter.interfaceCppType}} param_{{parameter.name}};
        deserializeValue(msg, param_{{parameter.name}});
        {% endfor %}
        emit {{event}}(
        {%- set comma = joiner(", ") -%}
        {%- for parameter in event.parameters -%}
            {{ comma() }}param_{{parameter.name}}
        {%- endfor -%}  );
    }    break;

    {% endfor %}
    {% for property in interface.properties %}
    case SignalID::{{property.name}}:
    {% if property.type.is_model %}
        m_{{property.name}}.handleSignal(msg);
    {% else %}
        emit {{property.name}}Changed();
    {% endif %}
        break;
    {% endfor %}
    default :
        BaseType::deserializeCommonSignal(static_cast<facelift::CommonSignalID>(member));
        break;
    }
}

{% for operation in interface.operations %}

{% if operation.isAsync %}
void {{className}}::{{operation.name}}(
    {%- for parameter in operation.parameters -%}{%if parameter.type.is_struct or parameter.type.is_enum %}const {{parameter.cppType}} &{% else %}{{parameter.cppType}} {% endif %}{{parameter.name}}, {% endfor %}facelift::AsyncAnswer<{{operation.interfaceCppType}}> answer){% if operation.is_const %} const{% endif %} {
        sendAsyncMethodCall(memberID(MethodID::{{operation.name}}, "{{operation.name}}"), answer
        {%- for parameter in operation.parameters -%}
        , {{parameter.name}}
        {%- endfor -%}  );
}
{% else %}
{{operation.interfaceCppType}} {{className}}::{{operation.name}}(
    {%- set comma = joiner(", ") -%}
    {%- for parameter in operation.parameters -%}
    {{ comma() }}{%if parameter.type.is_struct or parameter.type.is_enum %}const {{parameter.cppType}} &{% else %}{{parameter.cppType}} {% endif %}{{parameter.name}}
    {%- endfor -%}  ){% if operation.is_const %} const{% endif %}
{
        {% if (operation.hasReturnValue) %}
        {{operation.interfaceCppType}} returnValue;
        sendMethodCallWithReturn(memberID(MethodID::{{operation.name}}, "{{operation.name}}"), returnValue
            {%- for parameter in operation.parameters -%}
            , {{parameter.name}}
            {%- endfor -%} );
        return returnValue;
        {% else %}
        sendMethodCall(memberID(MethodID::{{operation.name}}, "{{operation.name}}")
        {%- for parameter in operation.parameters -%}
        , {{parameter.name}}
        {%- endfor -%}  );
        {% endif %}
}
{% endif %}
{% endfor %}


{{module.namespaceCppClose}}
