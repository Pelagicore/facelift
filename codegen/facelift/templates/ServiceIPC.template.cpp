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

#include "{{interfaceName}}IPC.h"

{{module.namespaceCppOpen}}


facelift::IPCHandlingResult {{interfaceName}}IPCAdapter::handleMethodCallMessage(facelift::IPCMessage &requestMessage,
                                                    facelift::IPCMessage &replyMessage)
{
    Q_UNUSED(replyMessage); // Since we do not always have return values
    Q_UNUSED(requestMessage);

    const auto &member = requestMessage.member();
    Q_UNUSED(member);  // In case there are no methods
    auto theService = service();
    {% if (not interface.operations) %}
    Q_UNUSED(theService);
    {% endif %}

    {% for operation in interface.operations %}
    if (member == memberID(MethodID::{{operation.name}}, "{{operation.name}}")) {
        {% for parameter in operation.parameters %}
        {{parameter.cppType}} param_{{parameter.name}};
        deserializeValue(requestMessage, param_{{parameter.name}});
        {% endfor %}
        {% if operation.isAsync %}
        theService->{{operation.name}}({% for parameter in operation.parameters %} param_{{parameter.name}}, {%- endfor -%}
            facelift::AsyncAnswer<{{operation.interfaceCppType}}>(this, [this, replyMessage] ({% if operation.hasReturnValue %} {{operation.interfaceCppType}} const & returnValue {% endif %}) mutable {
            sendAsyncCallAnswer(replyMessage{% if operation.hasReturnValue %}, returnValue{% endif %});
        }));
        return facelift::IPCHandlingResult::OK_ASYNC;
        {% else %}
        {% if operation.hasReturnValue %}
        auto returnValue =
        {%- endif %}
        theService->{{operation.name}}(
        {%- set comma = joiner(", ") -%}
        {%- for parameter in operation.parameters -%}
            {{ comma() }}param_{{parameter.name}}
        {%- endfor -%});
        {% if operation.hasReturnValue %}
        serializeValue(replyMessage, returnValue);
        {% endif %}
        {% endif %}
    } else
    {% endfor %}
    {% for property in interface.properties %}
    {% if property.type.is_model %}
    if (member == memberID(MethodID::{{property.name}}, "{{property.name}}")) {
        m_{{property.name}}Handler.handleModelRequest(requestMessage, replyMessage);
    } else
    {% endif %}
    {% if (not property.readonly) %}
    if (member == memberID(MethodID::set{{property.name}}, "set{{property.name}}")) {
        {% if (not property.type.is_interface) %}
        {{property.cppType}} value;
        deserializeValue(requestMessage, value);
        theService->set{{property.name}}(value);
        {% else %}
        Q_ASSERT(false); // Writable interface properties are unsupported
        {% endif %}
    } else
    {% endif %}
    {% endfor %}
    {
        return facelift::IPCHandlingResult::INVALID;
    }

    return facelift::IPCHandlingResult::OK;
}


void {{interfaceName}}IPCAdapter::appendDBUSIntrospectionData(QTextStream &s) const
{
    Q_UNUSED(s);   // For empty interfaces
    {% for property in interface.properties %}
    addPropertySignature<ServiceType::PropertyType_{{property.name}}>(s, "{{property.name}}", {{ property.readonly | cppBool }});
    {% endfor %}
    {% for operation in interface.operations %}

    {
        std::array<const char*, {{ operation.parameters.__len__() }}> argumentNames = { {
            {%- for parameter in operation.parameters -%}
            "{{parameter}}",
            {%- endfor -%}
        } };
        addMethodSignature<
        {%- set comma = joiner(", ") -%}
        {%- for parameter in operation.parameters -%}
            {{ comma() }}{{parameter.cppType}}
        {%- endfor -%}
        >(s, "{{operation.name}}", argumentNames);
    }
    {% endfor %}

    // signals
    {% for signal in interface.signals %}
    {
        std::array<const char*, {{ signal.parameters.__len__() }}> argumentNames = { {
            {%- for parameter in signal.parameters -%}
                "{{parameter}}",
            {%- endfor -%}
        }};
        addSignalSignature<
        {%- set comma = joiner(", ") -%}
        {%- for parameter in signal.parameters -%}
        {{ comma() }}{{parameter.interfaceCppType}}
        {%- endfor -%}
        >(s, "{{signal.name}}", argumentNames);
    }

    {% endfor %}
}


void {{interfaceName}}IPCAdapter::setService(QObject *srvc)
{
    BaseType::setService(srvc);

    auto theService = service();
    Q_UNUSED(theService);

    {% for property in interface.properties %}
    {% if property.type.is_model %}
    m_{{property.name}}Handler.connectModel(SignalID::{{property.name}}, theService->{{property.name}}());
    {% elif property.type.is_interface %}
    {% else %}
    m_previous{{property.name}} = theService->{{property.name}}();
    {% endif %}
    {% endfor %}
}


void {{interfaceName}}IPCAdapter::connectSignals()
{
    auto theService = service();
    Q_UNUSED(theService);

    // Properties
    {% for property in interface.properties %}
    {% if property.type.is_interface %}
    m_{{property.name}}.update(this, theService->{{property.name}}());
    QObject::connect(theService, &ServiceType::{{property.name}}Changed, this, [this, theService] () {
        m_{{property.name}}.update(this, theService->{{property.name}}());
    });
    {% endif %}
    QObject::connect(theService, &ServiceType::{{property.name}}Changed, this, [this] () {
        {{interfaceName}}IPCAdapter::sendSignal(SignalID::{{property.name}});
    });
    {% endfor %}

    // Signals
    {% for signal in interface.signals %}
    QObject::connect(theService, &ServiceType::{{signal}}, this, &{{interfaceName}}IPCAdapter::{{signal}});
    {% endfor %}
}


void {{interfaceName}}IPCAdapter::serializePropertyValues(facelift::IPCMessage& msg, bool isCompleteSnapshot)
{
    auto theService = service();
    {#% if (not interface.properties) %#}
    Q_UNUSED(theService);
    {#% endif %#}

    {% for property in interface.properties %}
    {% if property.type.is_interface %}

    serializeOptionalValue(msg, m_{{property.name}}.objectPath(), m_previous{{property.name}}ObjectPath, isCompleteSnapshot);

    {% elif property.type.is_model %}
    if (isCompleteSnapshot) {
        serializeValue(msg, theService->{{property.name}}().size());
    }
    {% else %}
    serializeOptionalValue(msg, theService->{{property.name}}(), m_previous{{property.name}}, isCompleteSnapshot);
    {% endif %}
    {% endfor %}

    BaseType::serializePropertyValues(msg, isCompleteSnapshot);
}


void {{interfaceName}}IPCProxy::deserializePropertyValues(facelift::IPCMessage &msg, bool isCompleteSnapshot)
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

void {{interfaceName}}IPCProxy::emitChangeSignals() {
{% for property in interface.properties %}
    emit {{property.name}}Changed();
{% endfor %}
    BaseType::emitChangeSignals();
}

void {{interfaceName}}IPCProxy::deserializeSignal(facelift::IPCMessage &msg)
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

void {{interfaceName}}IPCProxy::bindLocalService({{interfaceName}} *service)
{
    Q_UNUSED(service);

    // Bind all properties
    {% for property in interface.properties %}
    QObject::connect(service, &{{interfaceName}}::{{property.name}}Changed, this, &{{interfaceName}}::{{property.name}}Changed);
    {% endfor %}

    // Forward all signals
    {% for signal in interface.signals %}
    QObject::connect(service, &InterfaceType::{{signal.name}}, this, &InterfaceType::{{signal.name}});
    {% endfor %}
}

{{module.namespaceCppClose}}
