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

void {{className}}::unmarshalProperties(const QVariantMap& values)
{
    QMap<QString, bool> emitChangeSignal;
    for (const QString &propertyName: values.keys()) {
        {% for property in interface.properties %}
        if (propertyName == QStringLiteral("{{property.name}}")) {
        {% if property.type.is_interface %}
            const {{property.interfaceCppType}} previous_{{property.name}}_Value = m_{{property.name}};
            m_{{property.name}} = castFromQVariant<{{property.interfaceCppType}}>(values[propertyName]);
            emitChangeSignal[QStringLiteral("{{property.name}}")] = ((previous_{{property.name}}_Value != m_{{property.name}}));
        {% elif property.type.is_model %}
            emitChangeSignal[QStringLiteral("{{property.name}}")] = true;
            int {{property.name}}Size = castFromQVariant<int>(values[propertyName]);
            m_{{property.name}}.beginResetModel();
            m_{{property.name}}.reset({{property.name}}Size, std::bind(&ThisType::{{property.name}}Data, this, std::placeholders::_1));
            m_{{property.name}}.endResetModel();
        {% else %}
            const auto previous_{{property.name}}_Value = m_{{property.name}};
            m_{{property.name}} = castFromQVariant<{{property.interfaceCppType}}>(values[propertyName]);
            emitChangeSignal[QStringLiteral("{{property.name}}")] = ((previous_{{property.name}}_Value != m_{{property.name}}));
        {% endif %}
        }
        {% endfor %}
        if (propertyName == QStringLiteral("ready")) {
            bool previousIsReady = this->ready();
            m_serviceReady = castFromQVariant<bool>(values[propertyName]);
            emitChangeSignal[QStringLiteral("ready")] = (previousIsReady != m_serviceReady);
        }
    }
    for (const QString &propertyName: emitChangeSignal.keys()) {
        {% for property in interface.properties %}
        if (propertyName == QStringLiteral("{{property.name}}") && emitChangeSignal[propertyName]) {
            emit {{property.name}}Changed();
        }
        {% endfor %}
        if (propertyName == QStringLiteral("ready") && emitChangeSignal[propertyName]) {
            emit readyChanged();
        }
    }
}

void {{className}}::handleSignals(InputIPCMessage& msg)
{
    Q_UNUSED(msg)
    {% for event in interface.signals %}
    if (msg.member() == QStringLiteral("{{event}}")) {
        QListIterator<QVariant> argumentsIterator(msg.arguments());
        {% for parameter in event.parameters %}
        {{parameter.interfaceCppType}} param_{{parameter.name}};
        param_{{parameter.name}} = (argumentsIterator.hasNext() ? castFromQVariant<{{parameter.interfaceCppType}}>(argumentsIterator.next()):{% if not parameter.type.is_interface %}{{parameter.interfaceCppType}}(){% else %}nullptr{% endif %});
        {% endfor %}
        emit {{event}}(
        {%- set comma = joiner(", ") -%}
        {%- for parameter in event.parameters -%}
            {{ comma() }}param_{{parameter.name}}
        {%- endfor -%}  );
    }
    {% endfor %}

    {% if interface.hasModelProperty %}
    this->onModelUpdateEvent(msg);
    {% endif %}
}

const QList<QString>& {{className}}::getSignals() const
{
    static QList<QString> allSignals{
    {% for event in interface.signals %}
    "{{event}}",
    {% endfor %}
    {% if interface.hasModelProperty %}
    facelift::IPCCommon::MODEL_DATA_CHANGED_MESSAGE_NAME,
    facelift::IPCCommon::MODEL_INSERT_MESSAGE_NAME,
    facelift::IPCCommon::MODEL_REMOVE_MESSAGE_NAME,
    facelift::IPCCommon::MODEL_MOVE_MESSAGE_NAME,
    facelift::IPCCommon::MODEL_RESET_MESSAGE_NAME,
    {% endif %}
    };

    return allSignals;
}

{% if interface.hasModelProperty %}
void {{className}}::onModelUpdateEvent(const InputIPCMessage& msg)
{
    QListIterator<QVariant> argumentsIterator(msg.arguments());
    const QString& modelPropertyName = (argumentsIterator.hasNext() ? castFromQVariant<QString>(argumentsIterator.next()): QString());
    {% for property in interface.properties %}
    {% if property.type.is_model %}
    if (modelPropertyName == QStringLiteral("{{property.name}}")) {
        m_{{property.name}}.handleSignal(msg);
    }
    {% endif %}
    {% endfor %}
}
{% endif %}

void {{className}}::unmarshalPropertiesChanged(const QVariantMap& changedProperties)
{
    {% if interface.properties %}
    for (const QString &propertyName: changedProperties.keys()) {
        {% for property in interface.properties %}
        if (propertyName == QStringLiteral("{{property.name}}")) {
        {% if property.type.is_model %}
            int {{property.name}}Size = castFromQVariant<int>(changedProperties[propertyName]);
            m_{{property.name}}.beginResetModel();
            m_{{property.name}}.reset({{property.name}}Size, std::bind(&ThisType::{{property.name}}Data, this, std::placeholders::_1));
            m_{{property.name}}.endResetModel();
        {% else %}
            m_{{property.name}} = castFromQVariant<{{property.interfaceCppType}}>(changedProperties[propertyName]);
        {% endif %}
        }
        {% endfor %}
    }
    for (const QString &propertyName: changedProperties.keys()) {
        {% for property in interface.properties %}
        if (propertyName == QStringLiteral("{{property.name}}")) {
        {% if not property.type.is_model %}
            emit {{property.name}}Changed();
        {% endif %}
        }
        {% endfor %}
    }
    {% else %}
    Q_UNUSED(changedProperties);
    {% endif %}
}

{% for property in interface.properties %}

{% if (not property.readonly) %}

void {{className}}::set{{property}}({{property.cppMethodArgumentType}} newValue)
{
    {% if (not property.type.is_interface) %}
    ipc()->sendSetterCall("{{property.name}}", newValue);
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
        QList<QVariant> args = ipc()->sendMethodCallWithReturn(memberID(MethodID::{{operation.name}}, "{{operation.name}}")
            {%- for parameter in operation.parameters -%}
            , {{parameter.name}}
            {%- endfor -%} );
        return (!args.isEmpty() ? castFromQVariant<{{operation.interfaceCppType}}>(args.first()):{% if not (operation.type.is_interface) %}{{operation.cppType}}(){% else %}nullptr{% endif %});
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
