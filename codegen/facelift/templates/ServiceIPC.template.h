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

#include "ipc.h"

#include "{{interface|fullyQualifiedPath}}PropertyAdapter.h"
#include "{{interface|fullyQualifiedPath}}QMLFrontend.h"

{{module|namespaceOpen}}


class {{interface}}IPCQMLFrontendType;


class {{interface}}IPCAdapter: public facelift::IPCServiceAdapter<{{interface|fullyQualifiedCppName}}>
{
    Q_OBJECT

    // Q_PROPERTY(QObject* service READ service WRITE setService)
public:
    typedef {{interface|fullyQualifiedCppName}} ServiceType;

    static constexpr const char* IPC_SINGLETON_OBJECT_PATH = "/singletons/{{interface|fullyQualifiedName|lower|replace(".","/")}}";

    {{interface}}IPCAdapter(QObject* parent = nullptr)
        : facelift::IPCServiceAdapter<{{interface|fullyQualifiedCppName}}>(parent)
    {
        setObjectPath(IPC_SINGLETON_OBJECT_PATH);
    }

    void setService(facelift::InterfaceBase *srvc) override
    {
        facelift::IPCServiceAdapter<{{interface|fullyQualifiedCppName}}>::setService(srvc);

        {% for property in interface.properties %}
        {% if property.type.is_model %}
        facelift::ModelBase *{{property.name}} = &(service()->{{property.name}}());
        connect({{property.name}}, static_cast<void (facelift::ModelBase::*)(int, int)>
                (&facelift::ModelBase::dataChanged), this, [this] (int first, int last) {
            sendSignal("{{property.name}}DataChanged", first, last);
        });
        connect({{property.name}}, &facelift::ModelBase::beginRemoveElements, this, [this] (int first, int last) {
            sendSignal("{{property.name}}BeginRemove", first, last);
        });
        connect({{property.name}}, &facelift::ModelBase::endRemoveElements, this, [this] () {
            sendSignal("{{property.name}}EndRemove");
        });
        connect({{property.name}}, &facelift::ModelBase::beginInsertElements, this, [this] (int first, int last) {
            sendSignal("{{property.name}}BeginInsert", first, last);
        });
        connect({{property.name}}, &facelift::ModelBase::endInsertElements, this, [this] () {
            sendSignal("{{property.name}}EndInsert");
        });
        {% endif %}
        {% endfor %}
    }

    void appendDBUSIntrospectionData(QTextStream &s) const override
    {
        {% for property in interface.properties %}
        addPropertySignature<{{interface|fullyQualifiedCppName}}::PropertyType_{{property.name}}>(s, "{{property.name}}", {{ property.readonly | cppBool }});
        {% endfor %}
        {% for operation in interface.operations %}

        {
            std::array<const char*, {{ operation.parameters.__len__() }}> argumentNames = {
                {%- for parameter in operation.parameters -%}
                "{{parameter}}",
                {%- endfor -%}
            };
            addMethodSignature<
            {%- set comma = joiner(", ") -%}
            {%- for parameter in operation.parameters -%}
                {{ comma() }}{{parameter|returnType}}
            {%- endfor -%}
            >(s, "{{operation.name}}", argumentNames);
        }
        {% endfor %}

        // signals
        {% for signal in interface.signals %}
        {
            std::array<const char*, {{ signal.parameters.__len__() }}> argumentNames = {
                {%- for parameter in signal.parameters -%}
                    "{{parameter}}",
                {%- endfor -%}
            };
            addSignalSignature<
            {%- set comma = joiner(", ") -%}
            {%- for parameter in signal.parameters -%}
            {{ comma() }}{{parameter|returnType}}
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
            {{parameter|returnType}} param_{{parameter.name}};
            requestMessage >> param_{{parameter.name}};

            {% endfor %}
            {% if operation.hasReturnValue %}
            replyMessage << theService->{{operation.name}}(
            {%- else %}
            theService->{{operation.name}}(
            {%- endif %}
            {% set comma = joiner(", ") %}
            {% for parameter in operation.parameters -%}
                {{ comma() }}param_{{parameter.name}}
            {%- endfor -%}  );
        } else
        {% endfor %}
        {% for property in interface.properties %}
        {% if property.type.is_model %}
        if (member == "{{property.name}}Data") {
            int row;
            requestMessage >> row;
            replyMessage << theService->{{property.name}}().elementAt(row);
        } else
        {% endif %}
        {% if (not property.readonly) %}
        if (member == "set{{property.name}}") {
            {% if (not property.type.is_interface) %}
            {{property|returnType}} value;
            requestMessage >> value;
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

    void connectSignals() override
    {
        auto theService = service();
        {% if (not interface.properties and not interface.signals) %}
        Q_UNUSED(theService);
        {% endif %}

        // Properties
        {% for property in interface.properties %}
        connect(theService, &{{interface}}::{{property.name}}Changed, this, &{{interface}}IPCAdapter::onPropertyValueChanged);
        {% endfor %}

        // Signals
        {% for signal in interface.signals %}
        connect(theService, &{{interface}}::{{signal}}, this, &{{interface}}IPCAdapter::{{signal}});
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
        // TODO
        qWarning("Property of interface type not supported over IPC");
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
        {{ comma() }}{{parameter|returnType}} {{parameter.name}}
    {%- endfor -%}  )
    {
        sendSignal("{{event}}"
        {%- for parameter in event.parameters -%}
            , {{parameter.name}}
        {%- endfor -%}  );
    }
    {% endfor %}
};


class {{interface}}IPCProxy : public facelift::IPCProxy<{{interface}}PropertyAdapter, {{interface}}IPCAdapter>
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

        {% for property in interface.properties %}
        {% if property.type.is_model %}
        m_{{property.name}}.setGetter(std::bind(&{{interface}}IPCProxy::{{property.name}}Data, this, std::placeholders::_1));
        {% endif %}
        {% endfor %}
    }

    void deserializeSpecificPropertyValues(facelift::IPCMessage &msg) override
    {
        Q_UNUSED(msg);
        {% for property in interface.properties %}

        {% if property.type.is_interface %}
        // qFatal("Property of interface type not supported");
        qWarning() << "TODO: handle interface properties";
        {% elif property.type.is_model %}
        int {{property.name}}Size;
        msg >> {{property.name}}Size;
        if ({{property.name}}Size && !m_{{property.name}}.size()) {
            // we have an initial size, notify that size elements have been inserted
            emit m_{{property.name}}.beginInsertElements(0, {{property.name}}Size - 1);
            emit m_{{property.name}}.endInsertElements();
        }
        m_{{property.name}}.setSize({{property.name}}Size);
        {% else %}
        PropertyType_{{property.name}} {{property.name}};
        msg >> {{property.name}};
        m_{{property.name}}.setValue({{property.name}});
        {% endif %}
        {% endfor %}
    }

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
        {% for event in interface.signals %}

        if (signalName == "{{event}}") {
            {% for parameter in event.parameters %}
            {{parameter|returnType}} param_{{parameter.name}};
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
        if (signalName == "{{property.name}}DataChanged") {
            int first, last;
            msg >> first >> last;
            emit m_{{property.name}}.dataChanged(first, last);
        } else if (signalName == "{{property.name}}BeginInsert") {
            int first, last;
            msg >> first >> last;
            emit m_{{property.name}}.beginInsertElements(first, last);
        } else if (signalName == "{{property.name}}EndInsert") {
            emit m_{{property.name}}.endInsertElements();
        } else if (signalName == "{{property.name}}BeginRemove") {
            int first, last;
            msg >> first >> last;
            emit m_{{property.name}}.beginRemoveElements(first, last);
        } else if (signalName == "{{property.name}}EndRemove") {
            emit m_{{property.name}}.endRemoveElements();
        }
        {% endif %}
        {% endfor %}
    }
    {% for operation in interface.operations %}

    {{operation|returnType}} {{operation.name}}(
        {%- set comma = joiner(", ") -%}
        {%- for parameter in operation.parameters -%}
        {{ comma() }}{{ parameter|returnType }} {{ parameter.name }}
        {%- endfor -%}  ) override
    {
        if (localInterface() == nullptr) {
            {% if (operation.hasReturnValue) %}
            {{operation|returnType}} returnValue;
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
    {% endfor %}
    {%- for property in interface.properties %}

    {% if (not property.readonly) %}
    void set{{property}}(const {{property|returnType}}& newValue) override
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
    {{property|nestedType|fullyQualifiedCppName}} {{property.name}}Data(int row)
    {
        {{property|nestedType|fullyQualifiedCppName}} retval;
        sendMethodCallWithReturn("{{property.name}}Data", retval, row);
        return retval;
    }
    {% endif %}
    {% endfor %}
};


class {{interface}}IPCQMLFrontendType : public {{interface}}QMLFrontend
{
    Q_OBJECT

    Q_PROPERTY(facelift::IPCProxyBinderBase *ipc READ ipc CONSTANT)

public:
    {{interface}}IPCQMLFrontendType(QObject *parent = nullptr) : {{interface}}QMLFrontend(parent)
    {
    }

    facelift::IPCProxyBinder *ipc()
    {
        auto p = static_cast<{{interface}}IPCProxy*>(provider());
        return p->ipc();
    }
};

{{module|namespaceClose}}
