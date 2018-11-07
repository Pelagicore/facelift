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

{{classExportDefines}}

#pragma once

#include "facelift-ipc.h"
#include "{{module.fullyQualifiedPath}}/{{interfaceName}}.h"
#include "{{module.fullyQualifiedPath}}/{{interfaceName}}IPCAdapter.h"

{% for property in interface.referencedInterfaceTypes %}
#include "{{property.fullyQualifiedPath}}{% if generateAsyncProxy %}Async{% endif %}IPCProxy.h"
{% endfor %}

{{module.namespaceCppOpen}}

{% set className = interfaceName + "IPCProxy" %}

class {{className}}QMLFrontendType;

class {{classExport}} {{className}} : public facelift::IPCProxy<{{interfaceName}}, {{interfaceName}}IPCAdapter>
{
    Q_OBJECT

    Q_PROPERTY(facelift::IPCProxyBinderBase *ipc READ ipc CONSTANT)

public:
    using IPCAdapterType = {{interfaceName}}IPCAdapter;
    using ThisType = {{className}};
    using BaseType = facelift::IPCProxy<{{interfaceName}}, IPCAdapterType>;
    using SignalID = IPCAdapterType::SignalID;
    using MethodID = IPCAdapterType::MethodID;

    // override the default QMLFrontend type to add the IPC related properties
    using QMLFrontendType = {{className}}QMLFrontendType;

    {{className}}(QObject *parent = nullptr) : BaseType(parent)
        {% for property in interface.properties %}
        {% if property.type.is_interface %}
        , m_{{property.name}}Proxy(*this)
        {% endif %}
        {% if property.type.is_model %}
        , m_{{property.name}}(*this)
        {% endif %}
        {% endfor %}
    {
        ipc()->setObjectPath(IPCAdapterType::IPC_SINGLETON_OBJECT_PATH);
        {% if generateAsyncProxy %}
        ipc()->setSynchronous(false);
        {% endif %}
    }

    void deserializePropertyValues(facelift::IPCMessage &msg, bool isCompleteSnapshot) override;

    {% if interface.hasModelProperty %}
    void setServiceRegistered(bool isRegistered) override
    {
        if (isRegistered) {
        {% for property in interface.properties %}
        {% if property.type.is_model %}
            m_{{property.name}}.reset(m_{{property.name}}.size(), std::bind(&ThisType::{{property.name}}Data, this, std::placeholders::_1));
        {% endif %}
        {% endfor %}
        }
        BaseType::setServiceRegistered(isRegistered);
    }

    {% endif %}

    void bindLocalService({{interfaceName}} *service) override;

    void deserializeSignal(facelift::IPCMessage &msg) override;

    {% for operation in interface.operations %}

    {% if operation.isAsync %}
    void {{operation.name}}(
        {%- for parameter in operation.parameters -%}{{parameter.cppType}} {{parameter.name}}, {% endfor %}facelift::AsyncAnswer<{{operation.interfaceCppType}}> answer = facelift::AsyncAnswer<{{operation.interfaceCppType}}>()){% if operation.is_const %} const{% endif %} override {
        if (localInterface() == nullptr) {
            sendAsyncMethodCall(memberID(MethodID::{{operation.name}}, "{{operation.name}}"), answer
            {%- for parameter in operation.parameters -%}
            , {{parameter.name}}
            {%- endfor -%}  );
        } else {
            localInterface()->{{operation.name}}(
            {%- for parameter in operation.parameters -%}
                {{parameter.name}},
            {%- endfor -%} answer);
        }
    }
    {% else %}
    {{operation.interfaceCppType}} {{operation.name}}(
        {%- set comma = joiner(", ") -%}
        {%- for parameter in operation.parameters -%}
        {{ comma() }}{{ parameter.cppType }} {{ parameter.name }}
        {%- endfor -%}  ){% if operation.is_const %} const{% endif %} override
    {
        if (localInterface() == nullptr) {
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
        } else {
            {% if (operation.hasReturnValue) %}
            return localInterface()->{{operation.name}}(
            {%- else %}
            localInterface()->{{operation.name}}(
            {%- endif -%}
            {%- set comma = joiner(", ") -%}
            {%- for parameter in operation.parameters -%}
                {{ comma() }}{{parameter.name}}
            {%- endfor -%} );
        }
    }
    {% endif %}
    {% endfor %}
    {%- for property in interface.properties %}
    {% if (not property.readonly) %}

    void set{{property}}(const {{property.cppType}}& newValue) override
    {
        if (localInterface() == nullptr) {
            {% if (not property.type.is_interface) %}
            sendSetterCall(memberID(MethodID::set{{property.name}}, "set{{property.name}}"), newValue);
            {% else %}
            Q_ASSERT(false); // Writable interface properties are unsupported
            {% endif %}
        } else {
            localInterface()->set{{property}}(newValue);
        }
    }
    {% endif %}
    {% if property.type.is_model %}
    {{property.nestedType.interfaceCppType}} {{property.name}}Data(int row)
    {
        return m_{{property.name}}.modelData(memberID(MethodID::{{property.name}}, "{{property.name}}"), row);
    }
    {% endif %}
    {% endfor %}


    {% for property in interface.properties %}
    {% if property.type.is_model %}
    facelift::Model<{{property.nestedType.interfaceCppType}}>& {{property.name}}() override
    {
        return localInterface() ? localInterface()->{{property.name}}() : m_{{property.name}};
    }

    {% elif property.type.is_list %}

    const {{property.interfaceCppType}}& {{property}}() const override
    {
        return localInterface() ? localInterface()->{{property.name}}() : m_{{property.name}};
    }

    QList<{{property.nestedType.interfaceCppType}}> m_{{property.name}};

    {% elif property.type.is_interface -%}

    // Service property
    {{property.interfaceCppType}} {{property}}() override
    {
        return localInterface() ? localInterface()->{{property.name}}() : m_{{property.name}};
    }

    facelift::ServiceProperty<{{property.type.fullyQualifiedCppType}}> m_{{property.name}};

    {% else %}
    const {{property.interfaceCppType}} &{{property}}() const override
    {
        return localInterface() ? localInterface()->{{property.name}}() : m_{{property.name}};
    }
    {{property.interfaceCppType}} m_{{property.name}} = {};

    {% endif %}
    {% endfor %}

    void emitChangeSignals() override;

private:
    {% for property in interface.properties %}
    {% if property.type.is_interface %}
    InterfacePropertyIPCProxyHandler<{{property.cppType}}IPCProxy> m_{{property.name}}Proxy;
    {% endif %}
    {% if property.type.is_model %}
    facelift::IPCProxyModelProperty<ThisType, {{property.nestedType.interfaceCppType}}> m_{{property.name}};
    {% endif %}
    {% endfor %}
};


class {{classExport}} {{className}}QMLFrontendType : public {{interfaceName}}QMLFrontend
{
    Q_OBJECT

    Q_PROPERTY(facelift::IPCProxyBinderBase *ipc READ ipc CONSTANT)

public:
    {{className}}QMLFrontendType(QObject *parent = nullptr) : {{interfaceName}}QMLFrontend(parent)
    {
    }

    {{className}}QMLFrontendType(QQmlEngine *engine) : {{interfaceName}}QMLFrontend(engine)
    {
    }

    facelift::IPCProxyBinder *ipc()
    {
        auto p = static_cast<{{className}}*>(providerPrivate());
        return p->ipc();
    }
};


{{module.namespaceCppClose}}
