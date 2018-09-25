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

{{classExportDefines}}

#include "ipc.h"
#include "FaceliftUtils.h"

#include "{{module.fullyQualifiedPath}}/{{interfaceName}}PropertyAdapter.h"
#include "{{module.fullyQualifiedPath}}/{{interfaceName}}QMLFrontend.h"

{% for property in interface.properties %}
{{property.type.requiredIPCInclude}}
{% endfor %}

{{module.namespaceCppOpen}}


class {{interfaceName}}IPCQMLFrontendType;


class {{classExport}} {{interfaceName}}IPCAdapter: public facelift::IPCServiceAdapter<{{interfaceName}}>
{
    Q_OBJECT

    // Q_PROPERTY(QObject* service READ service WRITE setService)
public:

    enum class MethodID {
        {% for operation in interface.operations %}
        {{operation.name}},
        {% endfor %}
        {% for property in interface.properties %}
        {{property.name}},
        {% endfor %}
        {% for signal in interface.signals %}
        {{signal.name}},
        {% endfor %}
    };

    using ServiceType = {{interfaceName}};
    using BaseType = facelift::IPCServiceAdapter<{{interfaceName}}>;
    using ThisType = {{interfaceName}}IPCAdapter;

    static constexpr const char* IPC_SINGLETON_OBJECT_PATH = "/singletons/{{interface.qualified_name|lower|replace(".","/")}}";

    {{interfaceName}}IPCAdapter(QObject* parent = nullptr) : BaseType(parent)
    {% for property in interface.properties %}
    {% if property.type.is_model %}
        , m_{{property.name}}Handler(*this)
    {% endif %}
    {% endfor %}
    {
        setObjectPath(IPC_SINGLETON_OBJECT_PATH);
    }

    void setService(QObject *srvc) override
    {
        BaseType::setService(srvc);

        {% for property in interface.properties %}
        {% if property.type.is_model %}
        m_{{property.name}}Handler.connectModel(memberID(MethodID::{{property.name}}, "{{property.name}}"), service()->{{property.name}}());
        {% endif %}
        {% endfor %}
    }

    void appendDBUSIntrospectionData(QTextStream &s) const override
    {
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

    facelift::IPCHandlingResult handleMethodCallMessage(facelift::IPCMessage &requestMessage,
                                                        facelift::IPCMessage &replyMessage) override
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
            {%- endif %}
            {% endif %}
        } else
        {% endfor %}
        {% for property in interface.properties %}
        {% if property.type.is_model %}
        if (member == "{{property.name}}") {
            m_{{property.name}}Handler.handleModelRequest(requestMessage, replyMessage);
        } else
        {% endif %}
        {% if (not property.readonly) %}
        if (member == "set{{property.name}}") {
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

    {% for property in interface.properties %}{% if property.type.is_interface %}
    InterfacePropertyIPCAdapterHandler<{{property.cppType}}, {{property.cppType}}IPCAdapter> m_{{property.name}};
    {% endif %}
    {% endfor %}

    void connectSignals() override
    {
        // Properties
        {% for property in interface.properties %}
        {% if property.type.is_interface %}
        m_{{property.name}}.update(this, service()->{{property.name}}());
        QObject::connect(service(), &ServiceType::{{property.name}}Changed, [this] () {
            m_{{property.name}}.update(this, service()->{{property.name}}());
        });
        {% endif %}
        connect(service(), &ServiceType::{{property.name}}Changed, this, &{{interfaceName}}IPCAdapter::onPropertyValueChanged);
        {% endfor %}

        // Signals
        {% for signal in interface.signals %}
        connect(service(), &ServiceType::{{signal}}, this, &{{interfaceName}}IPCAdapter::{{signal}});
        {% endfor %}
    }

    void serializePropertyValues(facelift::IPCMessage& msg) override
    {
        auto theService = service();
        {#% if (not interface.properties) %#}
        Q_UNUSED(theService);
        {#% endif %#}

        {% for property in interface.properties %}
        {% if property.type.is_interface %}

        serializeValue(msg, m_{{property.name}}.objectPath());

        {% elif property.type.is_model %}
        serializeValue(msg, theService->{{property.name}}().size());
        {% else %}
        serializeValue(msg, theService->{{property.name}}());
        {% endif %}
        {% endfor %}

        BaseType::serializePropertyValues(msg);
    }

    {% for event in interface.signals %}

    void {{event}}(
    {%- set comma = joiner(", ") -%}
    {%- for parameter in event.parameters -%}
        {{ comma() }}{{parameter.interfaceCppType}} {{parameter.name}}
    {%- endfor -%}  )
    {
        sendSignal(memberID(MethodID::{{event}}, "{{event}}")
        {%- for parameter in event.parameters -%}
            , {{parameter.name}}
        {%- endfor -%}  );
    }
    {% endfor %}

private:
    {% for property in interface.properties %}
    {% if property.type.is_model %}
    facelift::IPCAdapterModelPropertyHandler<ThisType, {{property.nestedType.cppType}}> m_{{property.name}}Handler;
    {% endif %}
    {% endfor %}

};


{% set className = interfaceName + "IPCProxy" %}

class {{className}}QMLFrontendType;

class {{classExport}} {{className}} : public facelift::IPCProxy<{{interfaceName}}PropertyAdapter, {{interfaceName}}IPCAdapter>
{
    Q_OBJECT

    Q_PROPERTY(facelift::IPCProxyBinderBase *ipc READ ipc CONSTANT)

public:
    using IPCAdapterType = {{interfaceName}}IPCAdapter;
    using ThisType = {{className}};
    using BaseType = facelift::IPCProxy<{{interfaceName}}PropertyAdapter, IPCAdapterType>;
    using MethodID = IPCAdapterType::MethodID;

    // override the default QMLFrontend type to add the IPC related properties
    using QMLFrontendType = {{className}}QMLFrontendType;

    {{className}}(QObject *parent = nullptr) : BaseType(parent)
        {% for property in interface.properties %}
        {% if property.type.is_interface %}
        , m_{{property.name}}Proxy(*this)
        {% endif %}
        {% if property.type.is_model %}
        , m_{{property.name}}Handler(*this, m_{{property.name}})
        {% endif %}
        {% endfor %}
    {
        ipc()->setObjectPath(IPCAdapterType::IPC_SINGLETON_OBJECT_PATH);
        {% if generateAsyncProxy %}
        ipc()->setSynchronous(false);
        {% endif %}
    }

    void deserializePropertyValues(facelift::IPCMessage &msg) override
    {
        {% for property in interface.properties %}

        {% if property.type.is_interface %}
        QString {{property.name}}_objectPath;
        deserializeValue(msg, {{property.name}}_objectPath);
        m_{{property.name}}Proxy.update({{property.name}}_objectPath);
        m_{{property.name}} = m_{{property.name}}Proxy.getValue();
        {% elif property.type.is_model %}
        int {{property.name}}Size;
        deserializeValue(msg, {{property.name}}Size);
        m_{{property.name}}.beginResetModel();
        m_{{property.name}}.reset({{property.name}}Size, std::bind(&ThisType::{{property.name}}Data, this, std::placeholders::_1));
        m_{{property.name}}.endResetModel();
        {% else %}
        PropertyType_{{property.name}} {{property.name}};
        deserializeValue(msg, {{property.name}});
        m_{{property.name}}.setValue({{property.name}});
        {% endif %}
        {% endfor %}

        BaseType::deserializePropertyValues(msg);
    }

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
    void bindLocalService({{interfaceName}} *service) override
    {
        Q_UNUSED(service);

        // Bind all properties
        {% for property in interface.properties %}
        m_{{property.name}}.bind(service->{{property.name}}Property());
        {% endfor %}

        // Forward all signals
        {% for signal in interface.signals %}
        QObject::connect(service, &InterfaceType::{{signal.name}}, this, &InterfaceType::{{signal.name}});
        {% endfor %}
    }

    void deserializeSignal(facelift::IPCMessage &msg) override
    {
        QString signalName;
        deserializeValue(msg, signalName);

        {% for event in interface.signals %}
        if (signalName == "{{event}}") {
            {% for parameter in event.parameters %}
            {{parameter.interfaceCppType}} param_{{parameter.name}};
            deserializeValue(msg, param_{{parameter.name}});
            {% endfor %}
            {{event}}(
            {%- set comma = joiner(", ") -%}
            {%- for parameter in event.parameters -%}
                {{ comma() }}param_{{parameter.name}}
            {%- endfor -%}  );
        }

        {% endfor %}
        {% for property in interface.properties %}
        {% if property.type.is_model %}
        if (signalName == "{{property.name}}") {
            m_{{property.name}}Handler.handleSignal(msg);
        }
        {% endif %}
        {% endfor %}
    }
    {% for operation in interface.operations %}

    {% if operation.isAsync %}
    void {{operation.name}}(
        {%- for parameter in operation.parameters -%}{{parameter.cppType}} {{parameter.name}}, {% endfor %}facelift::AsyncAnswer<{{operation.interfaceCppType}}> answer){% if operation.is_const %} const{% endif %} override {
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
            sendSetterCall(memberID(MethodID::{{property.name}}, "set{{property.name}}"), newValue);
            {% else %}
            Q_ASSERT(false); // Writable interface properties are unsupported
            {% endif %}
        } else {
            localInterface()->set{{property}}(newValue);
        }
    }
    {% endif %}
    {% if property.type.is_model %}
    {{property.nestedType.cppType}} {{property.name}}Data(int row)
    {
        return m_{{property.name}}Handler.modelData(memberID(MethodID::{{property.name}}, "{{property.name}}"), row);
    }
    {% endif %}
    {% endfor %}

private:
    {% for property in interface.properties %}
    {% if property.type.is_interface %}
    InterfacePropertyIPCProxyHandler<{{property.cppType}}IPCProxy> m_{{property.name}}Proxy;
    {% endif %}
    {% if property.type.is_model %}
    facelift::IPCProxyModelPropertyHandler<ThisType, {{property.nestedType.cppType}}> m_{{property.name}}Handler;
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
        auto p = static_cast<{{className}}*>(provider());
        return p->ipc();
    }
};


{{module.namespaceCppClose}}
