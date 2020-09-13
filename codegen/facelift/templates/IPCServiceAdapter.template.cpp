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

#include <QtDBus>
#include "DBusIPCCommon.h"
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
        {{parameter.cppType}} param_{{parameter.name}} = (argumentsIterator.hasNext() ? castFromVariant<{{parameter.cppType}}>(argumentsIterator.next()):{% if not parameter.type.is_interface %}{{parameter.cppType}}(){% else %}nullptr{% endif %});
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
        replyMessage << castToVariant(returnValue);
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
    {% endfor %}
    {
        return facelift::IPCHandlingResult::INVALID;
    }

    return facelift::IPCHandlingResult::OK;
}

void {{className}}::appendDBUSIntrospectionData(QTextStream &s) const
{
    Q_UNUSED(s);   // For empty interfaces
    {% for property in interface.properties %}
    {% if not property.type.is_model %}
    s << QStringLiteral("<property name=\"{{property.name}}\" type=\"%1\" access=\"%2\"/>").arg(typeToSignature<{{property.type.interfaceCppType}}>(),
                               {{ property.readonly | cppBool }} ? QStringLiteral("read"): QStringLiteral("readwrite"));
    {% endif %}
    {% endfor %}
    s << QStringLiteral("<property name=\"ready\" type=\"b\" access=\"read\"/>");

    {% for operation in interface.operations %}
    s << "<method name=\"{{operation.name}}\">";
    {%- for parameter in operation.parameters -%}
    s << "<arg name=\"{{parameter.name}}\" type=\"";
    s << typeToSignature<{{parameter.type.interfaceCppType}}>();
    s << "\" direction=\"in\"/>";
    {%- endfor -%}

    {% if operation.hasReturnValue %}
    s << "<arg name=\"returnValue\" type=\"";
    s << typeToSignature<{{operation.interfaceCppType}}>();
    s << "\" direction=\"out\"/>";
    {% endif %};
    s << "</method>";
    {% endfor %}

    // signals
    {% for signal in interface.signals %}
    {
        s << "<signal name=\"{{signal.name}}\">";
        {%- for parameter in signal.parameters -%}
        s << "<arg name=\"{{parameter.name}}\" type=\"";
        s << typeToSignature<{{parameter.type.interfaceCppType}}>();
        s << "\" direction=\"out\"/>";
        {%- endfor -%}
        s << "</signal>";
    }
    {% endfor %}

    {% if interface.hasModelProperty %}
    s << "<signal name=\"ModelUpdateEventDataChanged\">";
    s << "<arg name=\"modelName\" type=\"s\" direction=\"out\"/>";
    s << "<arg name=\"first\" type=\"i\" direction=\"out\"/>";
    s << "<arg name=\"changedItems\" type=\"a(v)\" direction=\"out\"/>";
    s << "</signal>";
    {% endif %}
}

void {{className}}::connectSignals()
{
    auto theService = service();
    Q_UNUSED(theService);

    {% for property in interface.properties %}
    {% if property.type.is_model %}
    m_{{property.name}}Handler.connectModel(QStringLiteral("{{property.name}}"), theService->{{property.name}}());
    {% elif property.type.is_interface %}
    {% endif %}
    {% endfor %}

    // Properties
    {% for property in interface.properties %}
    {% if (not property.type.is_model) %}
    QObject::connect(theService, &ServiceType::{{property.name}}Changed, this, [this, theService] () {
        this->sendPropertiesChanged("{{property.name}}", theService->{{property.name}}());
    });
    {% endif %}
    {% endfor %}

    QObject::connect(theService, &ServiceType::readyChanged, this, [this, theService] () {
        this->sendPropertiesChanged("ready", theService->ready());
    });

    // Signals
    {% for signal in interface.signals %}
    QObject::connect(theService, &ServiceType::{{signal}}, this, &ThisType::{{signal}});
    {% endfor %}
}

void {{className}}::marshalPropertyValues(const QList<QVariant>& arguments, OutputIPCMessage& msg)
{
    QListIterator<QVariant> argumentsIterator(arguments);
    auto msgInterfaceName = (argumentsIterator.hasNext() ? castFromVariant<QString>(argumentsIterator.next()): QString());
    if (msgInterfaceName == interfaceName()) {
        auto theService = service();
        QMap<QString, QDBusVariant> ret;
        {#% if (not interface.properties) %#}
        Q_UNUSED(theService);
        {#% endif %#}

        {% for property in interface.properties %}
        {% if property.type.is_model %}
        ret["{{property.name}}"] = castToDBusVariant(theService->{{property.name}}().size());
        {% else %}
        ret["{{property.name}}"] = castToDBusVariant(theService->{{property.name}}());
        {% endif %}
        {% endfor %}
        ret["ready"] = castToDBusVariant(theService->ready());
        msg << castToVariant(ret);
    }
}

void {{className}}::marshalProperty(const QList<QVariant>& arguments, OutputIPCMessage& msg)
{
    QListIterator<QVariant> argumentsIterator(arguments);
    auto msgInterfaceName = (argumentsIterator.hasNext() ? castFromVariant<QString>(argumentsIterator.next()): QString());
    if (msgInterfaceName == interfaceName()) {
        auto propertyName = (argumentsIterator.hasNext() ? castFromVariant<QString>(argumentsIterator.next()): QString());
        {% for property in interface.properties %}
        if (propertyName == QStringLiteral("{{property.name}}")) {
        {% if property.type.is_model %}

        {% else %}
            msg << castToVariant(service()->{{property.name}}());
        {% endif %}
        }
        {% endfor %}
        if (propertyName == QStringLiteral("ready")) {
            msg << castToVariant(service()->ready());
        }
    }
}

void {{className}}::setProperty(const QList<QVariant>& arguments)
{
    QListIterator<QVariant> argumentsIterator(arguments);
    auto msgInterfaceName = (argumentsIterator.hasNext() ? castFromVariant<QString>(argumentsIterator.next()): QString());
    if (msgInterfaceName == interfaceName()) {
        auto propertyName = (argumentsIterator.hasNext() ? castFromVariant<QString>(argumentsIterator.next()): QString());
        if (argumentsIterator.hasNext()) {
            {% for property in interface.properties %}
            if (propertyName == QStringLiteral("{{property.name}}")) {
            {% if property.type.is_interface %}
                Q_ASSERT(false); // Writable interface properties are unsupported
            {% elif property.type.is_model %}

            {% elif (not property.readonly) %}
                service()->set{{property.name}}(castFromDBusVariant<{{property.cppType}}>(argumentsIterator.next()));
            {% endif %}
	    }
            {% endfor %}
        }
    }
}

{{module.namespaceCppClose}}
