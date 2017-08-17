/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include "ipc.h"

#include "{{interface|fullyQualifiedPath}}PropertyAdapter.h"
#include "{{interface|fullyQualifiedPath}}QMLFrontend.h"

{{module|namespaceOpen}}

class {{interface}}IPCAdapter: public facelift::IPCServiceAdapter<{{interface|fullyQualifiedCppName}}> {

    Q_OBJECT

public:

    static constexpr const char* IPC_SINGLETON_OBJECT_PATH = "/singletons/{{interface|fullyQualifiedName|lower|replace(".","/")}}";

//    Q_PROPERTY(QObject* service READ service WRITE setService)

    typedef {{interface|fullyQualifiedCppName}} ServiceType;

    {{interface}}IPCAdapter(QObject* parent = nullptr) : facelift::IPCServiceAdapter<{{interface|fullyQualifiedCppName}}>(parent)
    {
        setObjectPath(IPC_SINGLETON_OBJECT_PATH);
    }

    void appendDBUSIntrospectionData(QTextStream &s) const override {
        Q_UNUSED(s);
        {% for property in interface.properties %}
        {
            addPropertySignature<{{interface|fullyQualifiedCppName}}::PropertyType_{{property.name}}>(s, "{{property.name}}"
            , {{ property.readonly | cppBool }}
            );
        }
        {% endfor %}

        {% for operation in interface.operations %}
        {
    		std::array<const char*, {{ operation.parameters.__len__() }} > argumentNames = {
    	            {% for parameter in operation.parameters %}
    	            "{{parameter}}",
    	            {% endfor %}
    		};

            addMethodSignature<
            {% set comma = joiner(",") %}
            {% for parameter in operation.parameters %}
                {{ comma() }}
                {{parameter|returnType}}
            {% endfor %}
            >(s, "{{operation.name}}", argumentNames

            );
        }
        {% endfor %}

        // signals
        {% for signal in interface.signals %}
        {
		std::array<const char*, {{ signal.parameters.__len__() }} > argumentNames = {
	            {% for parameter in signal.parameters %}
	            "{{parameter}}",
	            {% endfor %}
		};

        addSignalSignature<
        {% set comma = joiner(",") %}
        {% for parameter in signal.parameters %}
            {{ comma() }}
            {{parameter|returnType}}
        {% endfor %}
        >(s, "{{signal.name}}", argumentNames

        );
        }
        {% endfor %}

    }

    facelift::IPCHandlingResult handleMethodCallMessage(facelift::IPCMessage& requestMessage, facelift::IPCMessage& replyMessage) override {

        Q_UNUSED(replyMessage); // Since we do not always have return values
        Q_UNUSED(requestMessage);

        const auto& member = requestMessage.member();

        {% for operation in interface.operations %}
        if (member == "{{operation.name}}") {
            {% for parameter in operation.parameters %}
            {{parameter|returnType}} param_{{parameter.name}};
            requestMessage >> param_{{parameter.name}};
            {% endfor %}

        	{% if (operation.hasReturnValue) %} auto returnValue = {% endif %}

            m_service->{{operation.name}}(
                    {% set comma = joiner(",") %}
                    {% for parameter in operation.parameters %}
                        {{ comma() }}
                        param_{{parameter.name}}
                    {% endfor %}
            );

         	{% if (operation.hasReturnValue) %} replyMessage << returnValue; {% endif %}

        } else
        {% endfor %}

        {% for property in interface.properties %}
    	{% if (not property.readonly) %}
        if (member == "set{{property.name}}") {
            {{property|returnType}} value;
            requestMessage >> value;
            m_service->set{{property.name}}(value);
        } else
    	{% endif %}
        {% endfor %}



        {
            return facelift::IPCHandlingResult::INVALID;
        }
        return facelift::IPCHandlingResult::OK;
    }

    void connectSignals() override {

        // Properties
        {% for property in interface.properties %}
        connect(m_service, &{{interface}}::{{property.name}}Changed, this, &{{interface}}IPCAdapter::onPropertyValueChanged);
        {% endfor %}

        // signals
        {% for signal in interface.signals %}
        connect(m_service, &{{interface}}::{{signal}}, this, &{{interface}}IPCAdapter::{{signal}});
        {% endfor %}

    }

    void serializeSpecificPropertyValues(facelift::IPCMessage& msg) override {
        Q_UNUSED(msg);
        {% for property in interface.properties %}
            {%if property.type.is_model -%}
            // TODO : model
//            qFatal("Property of model type not supported");
            qWarning() << "TODO";
            {% elif property.type.is_interface -%}
            // TODO
            qWarning() << "TODO";

//            qFatal("Property of interface type not supported");
            {% else %}
        msg << m_service->{{property.name}}();
            {% endif %}
        {% endfor %}
    }

    {% for event in interface.signals %}
        void {{event}}(
                {% set comma = joiner(",") %}
                {% for parameter in event.parameters %}
                    {{ comma() }}
                    {{parameter|returnType}} {{parameter.name}}
                    {% endfor %}
        ) {
            sendSignal("{{event}}"
            {% for parameter in event.parameters %}
                , {{parameter.name}}
                {% endfor %}
            );
        }
    {% endfor %}

};

class {{interface}}IPCQMLFrontendType;


class {{interface}}IPCProxy : public facelift::IPCProxy<{{interface}}PropertyAdapter, {{interface}}IPCAdapter> {

    Q_OBJECT

public:

	typedef {{interface}}IPCAdapter IPCAdapterType;

	// override the default QMLFrontend type to add teh IPC related properties
	typedef {{interface}}IPCQMLFrontendType QMLFrontendType;

    Q_PROPERTY(facelift::IPCProxyBinder* ipc READ ipc CONSTANT)

    {{interface}}IPCProxy(QObject* parent = nullptr) : facelift::IPCProxy<{{interface}}PropertyAdapter, IPCAdapterType>(parent) {
        ipc()->setObjectPath({{interface}}IPCAdapter::IPC_SINGLETON_OBJECT_PATH);
    }

    void deserializeSpecificPropertyValues(facelift::IPCMessage& msg) override {
        Q_UNUSED(msg);
        {% for property in interface.properties %}
        	{% if property.type.is_model -%}

        qFatal("Model not supported");
        	{% elif property.type.is_interface -%}
//        qFatal("Property of interface type not supported");
        qWarning() << "TODO : handle interface properties" ;

        	{% else %}
		PropertyType_{{property.name}} {{property.name}};
		msg >> {{property.name}};
		m_{{property.name}}.setValue({{property.name}});

            {% endif %}
        {% endfor %}
    }

    void bindLocalService({{interface}} * service) override {
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

    void deserializeSignal(facelift::IPCMessage& msg) override {

        QString signalName;
        msg >> signalName;

        {% for event in interface.signals %}
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
    	if (localInterface() == nullptr)
    	{
    	{% if (operation.hasReturnValue) %}
    	{{operation|returnType}} returnValue;
        sendMethodCallWithReturn("{{operation.name}}", returnValue
                {% for parameter in operation.parameters %}
                , {{parameter.name}}
                {% endfor %}
                );
        return returnValue;
        {% else %}
        sendMethodCall("{{operation.name}}"
                {% for parameter in operation.parameters %}
                , {{parameter.name}}
                {% endfor %}
                );
        {% endif %}
    	}
    	  	else return localInterface()->{{operation.name}}(
            		{% set comma = joiner(",") %}
    	  			{% for parameter in operation.parameters %}
    	  			{{ comma() }} {{parameter.name}}
                    {% endfor %}
         );

    }
    {% endfor %}


    {% for property in interface.properties %}
    	{% if (not property.readonly) %}
    void set{{property}}(const {{property|returnType}}& newValue) override {
    	if (localInterface() == nullptr) {
            sendMethodCall("set{{property}}", newValue);
        } else {
            localInterface()->set{{property}}(newValue);
        }
    }
        {% endif %}
    {% endfor %}


private:

};


class {{interface}}IPCQMLFrontendType : public {{interface}}QMLFrontend {

	Q_OBJECT

public:

    {{interface}}IPCQMLFrontendType(QObject* parent = nullptr) : {{interface}}QMLFrontend(parent) {
	}

    Q_PROPERTY(facelift::IPCProxyBinder* ipc READ ipc CONSTANT)

    facelift::IPCProxyBinder* ipc() {
    	auto p = static_cast<{{interface}}IPCProxy*>(provider());
    	return p->ipc();
    }

};

{{module|namespaceClose}}
