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

#include "{{interface.fullyQualifiedPath}}PropertyAdapter.h"
#include "{{interface.fullyQualifiedPath}}QMLFrontend.h"

{% for property in interface.properties %}
{{property.type.requiredIPCInclude}}
{% endfor %}

{{module.namespaceCppOpen}}


class {{interface}}IPCQMLFrontendType;


class {{classExport}} {{interface}}IPCAdapter: public facelift::IPCServiceAdapter<{{interface.fullyQualifiedCppType}}>
{
    Q_OBJECT

    // Q_PROPERTY(QObject* service READ service WRITE setService)
public:
    typedef {{interface.fullyQualifiedCppType}} ServiceType;

    static constexpr const char* IPC_SINGLETON_OBJECT_PATH = "/singletons/{{interface.qualified_name|lower|replace(".","/")}}";

    {{interface}}IPCAdapter(QObject* parent = nullptr)
        : facelift::IPCServiceAdapter<{{interface.fullyQualifiedCppType}}>(parent)
    {
        setObjectPath(IPC_SINGLETON_OBJECT_PATH);
    }

    void setService(facelift::InterfaceBase *srvc) override
    {
        facelift::IPCServiceAdapter<{{interface.fullyQualifiedCppType}}>::setService(srvc);

        {% for property in interface.properties %}
        {% if property.type.is_model %}
        connectModel("{{property.name}}", service()->{{property.name}}());
        {% endif %}
        {% endfor %}
    }

    void appendDBUSIntrospectionData(QTextStream &s) const override
    {
        {% for property in interface.properties %}
        addPropertySignature<{{interface.fullyQualifiedCppType}}::PropertyType_{{property.name}}>(s, "{{property.name}}", {{ property.readonly | cppBool }});
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
            {{ comma() }}{{parameter.cppType}}
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
        if (member == "{{operation.name}}") {
            {% for parameter in operation.parameters %}
            {{parameter.cppType}} param_{{parameter.name}};
            requestMessage >> param_{{parameter.name}};
            {% endfor %}
            {% if operation.isAsync %}
            facelift::ASyncRequestID requestID = s_nextRequestID++;
            theService->{{operation.name}}({% for parameter in operation.parameters %} param_{{parameter.name}}, {%- endfor -%}
                facelift::AsyncAnswer<{{operation.cppType}}>([this, requestID] ({% if operation.hasReturnValue %}const {{operation.cppType}}& returnValue {% endif %}) {
                sendSignal("{{operation.name}}Result", requestID{% if operation.hasReturnValue %}, returnValue{% endif %});
            }));
            replyMessage << requestID;
            {% else %}
            {% if operation.hasReturnValue %}
            replyMessage << theService->{{operation.name}}(
            {%- else %}
            theService->{{operation.name}}(
            {%- endif %}
            {%- set comma = joiner(", ") -%}
            {%- for parameter in operation.parameters -%}
                {{ comma() }}param_{{parameter.name}}
            {%- endfor -%});
            serializeSpecificPropertyValues(replyMessage);
            {% endif %}
        } else
        {% endfor %}
        {% for property in interface.properties %}
        {% if property.type.is_model %}
        if (member == "{{property.name}}Data") {
            handleModelRequest(theService->{{property.name}}(), requestMessage, replyMessage);
        } else
        {% endif %}
        {% if (not property.readonly) %}
        if (member == "set{{property.name}}") {
            {% if (not property.type.is_interface) %}
            {{property.cppType}} value;
            requestMessage >> value;
            theService->set{{property.name}}(value);
            serializeSpecificPropertyValues(replyMessage);
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
    facelift::InterfacePropertyIPCAdapterHandler<{{property.cppType}}, {{property.cppType}}IPCAdapter> m_{{property.name}};
    {% endif %}
    {% endfor %}

    void connectSignals() override
    {
        // Properties
        {% for property in interface.properties %}
        {% if property.type.is_interface %}
        m_{{property.name}}.update(this, service()->{{property.name}}());
        QObject::connect(service(), &{{interface}}::{{property.name}}Changed, [this] () {
            m_{{property.name}}.update(this, service()->{{property.name}}());
        });
        {% endif %}
        connect(service(), &{{interface}}::{{property.name}}Changed, this, &{{interface}}IPCAdapter::onPropertyValueChanged);
        {% endfor %}

        // Signals
        {% for signal in interface.signals %}
        connect(service(), &{{interface}}::{{signal}}, this, &{{interface}}IPCAdapter::{{signal}});
        {% endfor %}
    }

    void serializeSpecificPropertyValues(facelift::IPCMessage& msg) override
    {
        Q_UNUSED(msg);

        auto theService = service();
        {#% if (not interface.properties) %#}
        Q_UNUSED(theService);
        {#% endif %#}

        {% for property in interface.properties %}
        {% if property.type.is_interface %}

        msg << m_{{property.name}}.objectPath();

        {% elif property.type.is_model %}
        msg << theService->{{property.name}}().size();
        {% else %}
        msg << theService->{{property.name}}();
        {% endif %}
        {% endfor %}
    }
    {% for event in interface.signals %}

    void {{event}}(
    {%- set comma = joiner(", ") -%}
    {%- for parameter in event.parameters -%}
        {{ comma() }}{{parameter.cppType}} {{parameter.name}}
    {%- endfor -%}  )
    {
        sendSignal("{{event}}"
        {%- for parameter in event.parameters -%}
            , {{parameter.name}}
        {%- endfor -%}  );
    }
    {% endfor %}
};


class {{classExport}} {{interface}}IPCProxy : public facelift::IPCProxy<{{interface}}PropertyAdapter, {{interface}}IPCAdapter>
{
    Q_OBJECT

    Q_PROPERTY(facelift::IPCProxyBinderBase *ipc READ ipc CONSTANT)

public:
    typedef {{interface}}IPCAdapter IPCAdapterType;
    // override the default QMLFrontend type to add the IPC related properties
    typedef {{interface}}IPCQMLFrontendType QMLFrontendType;

    {{interface}}IPCProxy(QObject *parent = nullptr)
        : facelift::IPCProxy<{{interface}}PropertyAdapter
        , IPCAdapterType>(parent)
    {
        ipc()->setObjectPath({{interface}}IPCAdapter::IPC_SINGLETON_OBJECT_PATH);
    }

    void deserializeSpecificPropertyValues(facelift::IPCMessage &msg) override
    {
        Q_UNUSED(msg);
        {% for property in interface.properties %}

        {% if property.type.is_interface %}
        QString {{property.name}}_objectPath;
        msg >> {{property.name}}_objectPath;
        m_{{property.name}}Proxy.update({{property.name}}_objectPath);
        m_{{property.name}} = m_{{property.name}}Proxy.getValue();
        {% elif property.type.is_model %}
        int {{property.name}}Size;
        msg >> {{property.name}}Size;
        m_{{property.name}}.beginResetModel();
        m_{{property.name}}.reset({{property.name}}Size, std::bind(&{{interface}}IPCProxy::{{property.name}}Data, this, std::placeholders::_1));
        m_{{property.name}}.endResetModel();
        {% else %}
        PropertyType_{{property.name}} {{property.name}};
        msg >> {{property.name}};
        m_{{property.name}}.setValue({{property.name}});
        {% endif %}
        {% endfor %}
    }

    {% if interface.hasModelProperty %}
    void setServiceRegistered(bool isRegistered) override
    {
        if (isRegistered) {
        {% for property in interface.properties %}
        {% if property.type.is_model %}
            m_{{property.name}}.reset(m_{{property.name}}.size(), std::bind(&{{interface}}IPCProxy::{{property.name}}Data, this, std::placeholders::_1));
        {% endif %}
        {% endfor %}
        }
        facelift::IPCProxy<{{interface}}PropertyAdapter, {{interface}}IPCAdapter>::setServiceRegistered(isRegistered);
    }

    {% endif %}
    void bindLocalService({{interface}} *service) override
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
        msg >> signalName;

        {% for operation in interface.operations %}
        {% if operation.isAsync %}
        if (signalName == "{{operation.name}}Result") {
            facelift::ASyncRequestID id;
            msg >> id;
            if (m_{{operation.name}}Requests.contains(id)) {
                {% if operation.hasReturnValue %}
                {{operation.cppType}} returnValue;
                msg >> returnValue;
                m_{{operation.name}}Requests[id](returnValue);
                {% else %}
                m_{{operation.name}}Requests[id]();
                {% endif %}
                m_{{operation.name}}Requests.remove(id);
            }
        }

        {% endif %}
        {% endfor %}
        {% for event in interface.signals %}
        if (signalName == "{{event}}") {
            {% for parameter in event.parameters %}
            {{parameter.cppType}} param_{{parameter.name}};
            msg >> param_{{parameter.name}};
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
        handleModelSignal(m_{{property.name}}, m_{{property.name}}Cache, "{{property.name}}", signalName, msg);
        {% endif %}
        {% endfor %}
    }
    {% for operation in interface.operations %}

    {% if operation.isAsync %}
    void {{operation}}(
        {%- for parameter in operation.parameters -%}{{parameter.cppType}} {{parameter.name}}, {% endfor %}facelift::AsyncAnswer<{{operation.cppType}}> answer) override {
        if (localInterface() == nullptr) {
            facelift::ASyncRequestID id;
            sendMethodCallWithReturnNoSync("{{operation.name}}", id
            {%- for parameter in operation.parameters -%}
            , {{parameter.name}}
            {%- endfor -%}  );
            m_{{operation.name}}Requests.insert(id, answer);
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
        {%- endfor -%}  ) override
    {
        if (localInterface() == nullptr) {
            {% if (operation.hasReturnValue) %}
            {{operation.interfaceCppType}} returnValue;
            sendMethodCallWithReturn("{{operation.name}}", returnValue
                {%- for parameter in operation.parameters -%}
                , {{parameter.name}}
                {%- endfor -%} );
            return returnValue;
            {% else %}
            sendMethodCall("{{operation.name}}"
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
            sendSetterCall("set{{property}}", newValue);
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
        return modelData(m_{{property.name}}, m_{{property.name}}Cache, "{{property.name}}", row);
    }
    {% endif %}
    {% endfor %}

private:
    {% for operation in interface.operations %}
    {% if operation.isAsync %}
    QMap<facelift::ASyncRequestID, facelift::AsyncAnswer<{{operation.cppType}}>> m_{{operation.name}}Requests;
    {% endif %}
    {% endfor %}
    {% for property in interface.properties %}
    {% if property.type.is_interface %}
    facelift::InterfacePropertyIPCProxyHandler<{{property.cppType}}IPCProxy> m_{{property.name}}Proxy;
    {% endif %}
    {% if property.type.is_model %}
    facelift::MostRecentlyUsedCache<int, {{property.nestedType.cppType}}> m_{{property.name}}Cache;
    {% endif %}
    {% endfor %}
};


class {{classExport}} {{interface}}IPCQMLFrontendType : public {{interface}}QMLFrontend
{
    Q_OBJECT

    Q_PROPERTY(facelift::IPCProxyBinderBase *ipc READ ipc CONSTANT)

public:
    {{interface}}IPCQMLFrontendType(QObject *parent = nullptr) : {{interface}}QMLFrontend(parent)
    {
    }

    {{interface}}IPCQMLFrontendType(QQmlEngine *engine) : {{interface}}QMLFrontend(engine)
    {
    }

    facelift::IPCProxyBinder *ipc()
    {
        auto p = static_cast<{{interface}}IPCProxy*>(provider());
        return p->ipc();
    }
};

{{module.namespaceCppClose}}
