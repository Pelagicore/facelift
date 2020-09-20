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

#include "ipc-common.h"
#include "DBusSignatureHelper.h"
#include "{{className}}.h"

{{module.namespaceCppOpen}}

facelift::IPCHandlingResult {{className}}::handleMethodCallMessage(InputIPCMessage &requestMessage,
        OutputIPCMessage &replyMessage)
{
    Q_UNUSED(replyMessage); // Since we do not always have return values
    Q_UNUSED(requestMessage);
    QListIterator<QVariant> argumentsIterator(requestMessage.arguments());
    const auto &member = requestMessage.member();
    Q_UNUSED(member);  // In case there are no methods
    auto theService = service();
    {% if (not interface.operations) %}
    Q_UNUSED(theService);
    {% endif %}

    {% for operation in interface.operations %}
    if (member == memberID(MethodID::{{operation.name}}, "{{operation.name}}")) {
        {% for parameter in operation.parameters %}
        {{parameter.cppType}} param_{{parameter.name}} = (argumentsIterator.hasNext() ? castFromQVariant<{{parameter.cppType}}>(argumentsIterator.next()):{% if not parameter.type.is_interface %}{{parameter.cppType}}(){% else %}nullptr{% endif %});
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
        replyMessage << castToQVariant(returnValue);
        {% endif %}
        {% endif %}
    } else
    {% endfor %}
    {% for property in interface.properties %}
    {% if property.type.is_model %}
    if (member == memberID(MethodID::{{property.name}}, "{{property.name}}")) {
        QListIterator<QVariant> argumentsIterator(requestMessage.arguments());
        int first = (argumentsIterator.hasNext() ? castFromQVariant<int>(argumentsIterator.next()): int());
        int last = (argumentsIterator.hasNext() ? castFromQVariant<int>(argumentsIterator.next()): int());
        m_{{property.name}}Handler.handleModelRequest(first, last, replyMessage);
    } else
    {% endif %}
    {% endfor %}
    {
        return facelift::IPCHandlingResult::INVALID;
    }

    return facelift::IPCHandlingResult::OK;
}

void {{className}}::appendDBUSIntrospectionData(QTextStream &s) const
{
    Q_UNUSED(s)   // For empty interfaces
    {% for property in interface.properties %}
    {% if not property.type.is_model %}
    facelift::appendPropertySignature(s, "{{property.name}}", typeToSignature<{{property.type.interfaceCppType}}>(), {{ property.readonly | cppBool }});
    {% endif %}
    {% endfor %}
    facelift::appendReadyProperty(s);

    {% for operation in interface.operations %}
    facelift::appendDBusMethodSignature(s, "{{operation.name}}", std::list<std::pair<const char*, const char*>>{
    {%- set comma = joiner(", ") -%}
    {% for parameter in operation.parameters %}
    {{ comma() }}{"{{parameter.name}}", typeToSignature<{{parameter.type.interfaceCppType}}>()}
    {% endfor %}
    }
    {% if operation.hasReturnValue %}
    , typeToSignature<{{operation.interfaceCppType}}>()
    {% endif %}
    );
    {% endfor %}

    // signals
    {% for signal in interface.signals %}
    {
    facelift::appendDBusSignalSignature(s, "{{signal.name}}", std::list<std::pair<const char*, const char*>>{
    {%- set comma = joiner(", ") -%}
    {% for parameter in signal.parameters %}
    {{ comma() }}{"{{parameter.name}}", typeToSignature<{{parameter.type.interfaceCppType}}>()}
    {% endfor %}
    });
    }
    {% endfor %}

    {% if interface.hasModelProperty %}
    facelift::appendDBusModelSignals(s);
    {% endif %}
}

void {{className}}::connectSignals()
{
    auto theService = service();
    Q_UNUSED(theService);

    {% for property in interface.properties %}
    {% if property.type.is_model %}
    m_{{property.name}}Handler.connectModel(QStringLiteral("{{property.name}}"), theService->{{property.name}}());
    {% else %}
    m_previous{{property.name}} = theService->{{property.name}}();
    {% endif %}
    {% endfor %}
    m_previousReadyState = theService->ready();

    // Properties
    {% for property in interface.properties %}
    {% if (not property.type.is_model) %}
    QObject::connect(theService, &ServiceType::{{property.name}}Changed, this, [this, theService] () {
        this->sendPropertiesChanged(changedProperties());
    });
    {% endif %}
    {% endfor %}

    QObject::connect(theService, &ServiceType::readyChanged, this, [this, theService] () {
        this->sendPropertiesChanged(changedProperties());
    });

    // Signals
    {% for signal in interface.signals %}
    QObject::connect(theService, &ServiceType::{{signal}}, this, &ThisType::{{signal}});
    {% endfor %}
}

QVariantMap {{className}}::changedProperties()
{
    QMap<QString, QVariant> ret;
    auto theService = service();
    Q_UNUSED(theService);
    {% for property in interface.properties %}
    {% if not property.type.is_model %}
    if (m_previous{{property.name}} != theService->{{property.name}}()) {
        ret[QStringLiteral("{{property.name}}")] = castToQVariant(theService->{{property.name}}());
        m_previous{{property.name}} = theService->{{property.name}}();
    }
    {% endif %}
    {% endfor %}
    if (m_previousReadyState != theService->ready()) {
        ret[QStringLiteral("ready")] = castToQVariant(theService->ready());
        m_previousReadyState = theService->ready();
    }
    return ret;
}

QVariantMap {{className}}::marshalProperties()
{
    QVariantMap ret;
    auto theService = service();
    {#% if (not interface.properties) %#}
    Q_UNUSED(theService);
    {#% endif %#}

    {% for property in interface.properties %}
    {% if property.type.is_model %}
    ret["{{property.name}}"] = castToQVariant(theService->{{property.name}}().size());
    {% else %}
    ret["{{property.name}}"] = castToQVariant(theService->{{property.name}}());
    {% endif %}
    {% endfor %}
    ret["ready"] = castToQVariant(theService->ready());
    return ret;
}

QVariant {{className}}::marshalProperty(const QString& propertyName)
{
    {% for property in interface.properties %}
    if (propertyName == QStringLiteral("{{property.name}}")) {
    {% if property.type.is_model %}

    {% else %}
        return castToQVariant(service()->{{property.name}}());
    {% endif %}
    }
    {% endfor %}
    if (propertyName == QStringLiteral("ready")) {
        return castToQVariant(service()->ready());
    }
return QVariant();
}

void {{className}}::setProperty(const QString& propertyName, const QVariant& value)
{
    Q_UNUSED(propertyName)
    Q_UNUSED(value)
    {% for property in interface.properties %}
    if (propertyName == QStringLiteral("{{property.name}}")) {
    {% if property.type.is_interface %}
        Q_ASSERT(false); // Writable interface properties are unsupported
    {% elif property.type.is_model %}

    {% elif (not property.readonly) %}
        service()->set{{property.name}}(castFromQVariant<{{property.cppType}}>(value));
    {% endif %}
    }
    {% endfor %}
}

{{module.namespaceCppClose}}
