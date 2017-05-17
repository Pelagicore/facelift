/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include "ipc/ipc.h"

#include "{{interface|fullyQualifiedPath}}PropertyAdapter.h"
#include "{{interface|fullyQualifiedPath}}QMLFrontend.h"

{{module|namespaceOpen}}



class {{interface}}IPCAdapter: public IPCServiceAdapter<{{interface|fullyQualifiedCppName}}> {

    Q_OBJECT

public:

    Q_PROPERTY(QObject* service READ service WRITE setService);

    typedef {{interface|fullyQualifiedCppName}} ServiceType;

    {{interface}}IPCAdapter() {
        setObjectPath(ServiceType::IPC_SINGLETON_OBJECT_PATH);
    }

    void introspect(QTextStream &s) const override {
        Q_UNUSED(s);
        {% for property in interface.properties %}
//        addPropertySignature(m_service->{{property.name}}(), "{{property.name}}");
        {% endfor %}

        {% for operation in interface.operations %}

            addMethodSignature<
            {% set comma = joiner(",") %}
            {% for parameter in operation.parameters %}
                {{ comma() }}
                {{parameter|returnType}}
            {% endfor %}
            >(s, "{{operation.name}}"
            {% for parameter in operation.parameters %}
                ",{{parameter.name}}"

            {% endfor %}

            );

        {% endfor %}
}

/*
    void init(QDBusConnection& bus) {
        IPCServiceAdapterBase::init(bus, objectPath(m_service), {{interface}}IPCAdapter::INTERFACE);
    }
*/

    IPCHandlingResult handleMethodCallMessage(IPCMessage& requestMessage, IPCMessage& replyMessage) override {

        Q_UNUSED(replyMessage); // Since we do not always have return values
        Q_UNUSED(requestMessage);

        if (false) {}
        {% for operation in interface.operations %}
        else if (requestMessage.member() == "{{operation.name}}") {
            {% for parameter in operation.parameters %}
            {{parameter|returnType}} param_{{parameter.name}};
            requestMessage >> param_{{parameter.name}};
            {% endfor %}

            m_service->{{operation.name}}(
                    {% set comma = joiner(",") %}
                    {% for parameter in operation.parameters %}
                        {{ comma() }}
                        param_{{parameter.name}}
                    {% endfor %}
            );
        }
        {% endfor %}
        else {
            return IPCHandlingResult::INVALID;
        }
        return IPCHandlingResult::OK;
    }

    void connectSignals() override {

        // Properties
        {% for property in interface.properties %}
        connect(m_service, &{{interface}}::{{property.name}}Changed, this, &{{interface}}IPCAdapter::onPropertyValueChanged);
        {% endfor %}

        // signals
        {% for event in interface.events %}
        connect(m_service, &{{interface}}::{{event}}, this, &{{interface}}IPCAdapter::{{event}});
        {% endfor %}

    }

    void serializePropertyValues(IPCMessage& msg) override {
        Q_UNUSED(msg);
        {% for property in interface.properties %}
            {%if property.type.is_model -%}
            // TODO : model
            {% else %}
        msg << m_service->{{property.name}}();
            {% endif %}
        {% endfor %}
    }

    {% for event in interface.events %}
        void {{event}}(
                {% set comma = joiner(",") %}
                {% for parameter in event.parameters %}
                    {{ comma() }}
                    {{parameter|returnType}} {{parameter.name}}
                    {% endfor %}
        ) {
            sendSignal("{{event}}",
            {% set comma = joiner(",") %}
            {% for parameter in event.parameters %}
                {{ comma() }}
                {{parameter.name}}
                {% endfor %}
            );
        }
    {% endfor %}

};

class {{interface}}IPCProxy : public IPCProxy<{{interface}}PropertyAdapter> {

    Q_OBJECT

public:

    Q_PROPERTY(IPCProxyBinder* ipc READ ipc CONSTANT)

    {{interface}}IPCProxy(QObject* parent = nullptr) : IPCProxy<{{interface}}PropertyAdapter>(parent) {
        ipc()->setObjectPath({{interface}}PropertyAdapter::IPC_SINGLETON_OBJECT_PATH);
    }

/*
    void init(QDBusConnection& bus) {
        IPCProxyBase::init(bus, SERVICE, objectPath(this), {{interface}}::IPC_INTERFACE_NAME);
    }
*/

    void deserializePropertyValues(IPCMessage& msg) override {
        Q_UNUSED(msg);
        {% for property in interface.properties %}
            {%if property.type.is_model -%}
            {% else %}
            {
                typeof(m_{{property.name}}.value()) value;
                msg >> value;
                m_{{property.name}}.setValue(value);
            }
            {% endif %}
        {% endfor %}
    }

    void deserializeSignal(IPCMessage& msg) override {

        QString signalName;
        msg >> signalName;

        {% for event in interface.events %}
        if (signalName == "{{event}}") {
            {% for parameter in event.parameters %}
                {{parameter|returnType}} param_{{parameter.name}};
                msg >> param_{{parameter.name}};
            {% endfor %}

            {{event}}(
                {% set comma = joiner(",") %}
                {% for parameter in event.parameters %}
                    {{ comma() }}
                    param_{{parameter.name}}
                {% endfor %}
            );
        }
        {% endfor %}
    }

    {% for operation in interface.operations %}

    {{operation|returnType}} {{operation.name}}(
            {% set comma = joiner(",") %}
            {% for parameter in operation.parameters %}
            {{ comma() }}
            {{parameter|returnType}} {{parameter.name}}
            {% endfor %}
    ) override {
        sendMethodCall("{{operation.name}}"
                {% for parameter in operation.parameters %}
                , {{parameter.name}}
                {% endfor %}
                );
    }

    {% endfor %}

    {% for property in interface.properties %}
    	{% if (not property.is_readonly) %}
    virtual void set{{property}}({{property|returnType}} newValue) {
    	qDebug() << "TODO";
    }
        {% endif %}
    {% endfor %}


private:

};


{{module|namespaceClose}}
