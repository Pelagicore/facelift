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

#pragma once

{% set className = interfaceName + proxyTypeNameSuffix %}

{{classExportDefines}}

#include "FaceliftUtils.h"
#include "IPCAdapterModelPropertyHandler.h"

#include "{{module.fullyQualifiedPath}}/{{interfaceName}}.h"
#include "{{module.fullyQualifiedPath}}/{{interfaceName}}QMLAdapter.h"
#include "{{module.fullyQualifiedPath}}/{{interfaceName}}IPCCommon.h"

//// Sub interfaces
{% for property in interface.referencedInterfaceTypes %}
#include "{{property.fullyQualifiedPath}}{% if generateAsyncProxy %}Async{% endif %}{{proxyTypeNameSuffix}}.h"
{% endfor %}

{{module.namespaceCppOpen}}

class {{interfaceName}}IPCQMLAdapterType;

class {{classExport}} {{className}}: public {{baseClass}}
{
    Q_OBJECT

public:

    using ServiceType = {{interfaceName}};
    using BaseType = {{baseClass}};
    using ThisType = {{className}};
    using MethodID = {{interface}}IPCCommon::MethodID;

    {{className}}(QObject* parent = nullptr) : BaseType(parent)
    {% for property in interface.properties %}
    {% if property.type.is_model %}
        , m_{{property.name}}Handler(*this)
    {% endif %}
    {% endfor %}
    {
    }

    void appendDBUSIntrospectionData(QTextStream &s) const override;

    ::facelift::IPCHandlingResult handleMethodCallMessage(InputIPCMessage &requestMessage,
            OutputIPCMessage &replyMessage) override;

    void connectSignals() override;

    QVariantMap changedProperties();

    QVariantMap marshalProperties() override;

    QVariant marshalProperty(const QString& propertyName) override;

    void setProperty(const QString& propertyName, const QVariant& value) override;

    {% for event in interface.signals %}
    void {{event}}(
    {%- set comma = joiner(", ") -%}
    {%- for parameter in event.parameters -%}
        {{ comma() }}{{parameter.interfaceCppType}} {{parameter.name}}
    {%- endfor -%}  )
    {
        sendSignal("{{event}}"
        {%- for parameter in event.parameters -%}
            , {{parameter.name}}
        {%- endfor -%}  );
    }
    {% endfor %}

private:
    {% for property in interface.properties %}
    {% if property.type.is_model %}
    ::facelift::IPCAdapterModelPropertyHandler<ThisType, {{property.nestedType.interfaceCppType}}> m_{{property.name}}Handler;
    {% else %}
    {{property.interfaceCppType}} m_previous{{property.name}} {};
    {% endif %}
    {% endfor %}
    bool m_previousReadyState = false;
};

{{module.namespaceCppClose}}
