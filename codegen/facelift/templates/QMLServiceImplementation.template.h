/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include "QMLModel.h"

#include "{{interface}}PropertyAdapter.h"

{{module|namespaceOpen}}

class {{interface}}QMLImplementation;

/**
 * This class implements the actual service interface and wraps the object instantiated from QML, which implements
 * the actual logic
 */
class {{interface}}QMLImplementationFrontend : public {{interface}}PropertyAdapter, public QMLModelImplementationFrontend<{{interface}}QMLImplementation> {

    Q_OBJECT

public:
    typedef {{interface}}QMLImplementation QMLImplementationModelType;

    {{interface}}QMLImplementationFrontend({{interface}}QMLImplementation* qmlImpl);

    {{interface}}QMLImplementationFrontend();

    {% for operation in interface.operations %}
    {{operation|returnType}} {{operation.name}}(
            {% set comma = joiner(",") %}
            {% for parameter in operation.parameters %}
            {{ comma() }}
            {{parameter|returnType}} {{parameter.name}}
            {% endfor %}
    ) override;
    {% endfor %}

    {% for property in interface.properties %}
    	{% if (not property.readonly) %}
    void set{{property}}(const {{property|returnType}}& newValue) override;
        {% endif %}
    {% endfor %}

    static void registerTypes(const char* uri) {
        ModelQMLImplementation<{{interface}}QMLImplementationFrontend>::registerTypes(uri);
    }

    QObject* impl();

};

/**
 * This class defines the QML component which is used when implementing a model using QML
 */
class {{interface}}QMLImplementation : public ModelQMLImplementation<{{interface}}QMLImplementationFrontend> {

    Q_OBJECT

public:

    static constexpr const char* QML_NAME = "{{interface}}Implementation";

    typedef {{interface}}QMLImplementationFrontend Provider;
    typedef {{interface}}QMLImplementation ThisType;

    Q_PROPERTY(QObject* provider READ provider CONSTANT)

    {{interface}}QMLImplementationFrontend* createFrontend() override {
    	return new {{interface}}QMLImplementationFrontend(this);
    }

    void initProvider(Provider* provider) override {
    	Q_UNUSED(provider);
        {% for property in interface.properties %}
        QObject::connect(provider, &Provider::{{property}}Changed, this, &ThisType::{{property}}Changed);
        {% endfor %}
    }

    {% for operation in interface.operations %}

    {{operation|returnType}} {{operation.name}}(
        {% set comma = joiner(",") %}
        {% for parameter in operation.parameters %}
        {{ comma() }}
        {{parameter|returnType}} {{parameter.name}}
        {% endfor %}
    ) {

        QJSValueList args;

        QQmlEngine* engine = qmlEngine(this);

        Q_UNUSED(engine);

        {% for parameter in operation.parameters %}
        args.append(toJSValue({{parameter.name}}, engine));
        {% endfor %}

    	{% if (operation.hasReturnValue) %} {{operation|returnType}} returnValue; {% endif %}

    	auto jsReturnValue = checkMethod(m_{{operation}}, "{{operation}}").call(args);
		{% if (operation.hasReturnValue) %}
		fromJSValue(returnValue, jsReturnValue, engine);
		{% endif %}

        {% if (operation.hasReturnValue) %}
        return returnValue;
        {% endif %}
    }

    Q_PROPERTY(QJSValue {{operation.name}} READ {{operation.name}}JSFunction WRITE set{{operation.name}}JSFunction)

    void set{{operation.name}}JSFunction(QJSValue v) {
        m_{{operation.name}} = v;
    }

    QJSValue {{operation.name}}JSFunction() const {
        return m_{{operation.name}};
    }

    QJSValue m_{{operation.name}};

    {% endfor %}

    {% for property in interface.properties %}
    Q_SIGNAL void {{property}}Changed();
    {% endfor %}

    {% for property in interface.properties %}

    {% if property.type.is_list -%}
    Q_PROPERTY(QObject* {{property.name}} READ {{property.name}} NOTIFY {{property.name}}Changed)

    QMLImplListProperty<{{property|nestedType|fullyQualifiedCppName}}> m_{{property.name}}QMLProperty;


    QObject* {{property.name}}() {
    	m_{{property.name}}QMLProperty.setProperty(interface().m_{{property.name}});
        return &m_{{property.name}}QMLProperty;
    }

    {% elif property.type.is_model %}
    // TODO : model

    {% elif property.type.is_interface -%}
    // TODO : interface

    {% elif property.type.is_struct -%}

    Q_PROPERTY({{property|returnType}}QMLWrapper* {{property.name}} READ {{property.name}} WRITE set{{property.name}} NOTIFY {{property.name}}Changed)
    {{property|returnType}}QMLWrapper* {{property.name}}() const {
        return m_{{property.name}};
    }

    void set{{property.name}}({{property|returnType}}QMLWrapper* value) {
    	if (m_{{property.name}}.data() != value) {
			m_{{property.name}} = value;
			sync{{property.name}}();
			if (m_{{property.name}} != nullptr) {
				QObject::connect(m_{{property.name}}, &{{property|returnType}}QMLWrapper::anyFieldChanged, this, &{{interface}}QMLImplementation::sync{{property.name}});
			}
			emit {{property.name}}Changed();
//			qDebug() << "-----" << value->gadget();
    	}
    }

    void sync{{property.name}}() {
        checkInterface();
        {{property|returnType}} value;
        if (m_{{property.name}} != nullptr) {
        	value = m_{{property.name}}->gadget();
        }
        interface().m_{{property.name}} = value;
    }

    QPointer<{{property|returnType}}QMLWrapper> m_{{property.name}} = nullptr;

    {% else %}

		{% set QmlType=property|returnType %}
		{% if property.type.is_enum %}
			{% set QmlType=QmlType + "Gadget::Type" %}
		{% endif %}

      Q_PROPERTY({{QmlType}} {{property.name}} READ {{property.name}} WRITE set{{property.name}} NOTIFY {{property.name}}Changed)
      const {{QmlType}}& {{property.name}}() const {
          checkInterface();
          return interface().m_{{property.name}};
      }

      void set{{property.name}}(const {{QmlType}}& value) {
          checkInterface();
          interface().m_{{property.name}} = value;
      }

    {% endif %}

	{% if (not property.readonly) %}

    Q_PROPERTY(QJSValue set{{property.name}} READ set{{property.name}}JSFunction WRITE setset{{property.name}}JSFunction)

    void setset{{property.name}}JSFunction(QJSValue v) {
        m_set{{property.name}} = v;
    }

    QJSValue set{{property.name}}JSFunction() const {
        return m_set{{property.name}};
    }

    QJSValue m_set{{property.name}};


    void requestSet{{property.name}}(const {{property|returnType}}& value) {
        checkInterface();
        QJSValueList args;

        QQmlEngine* engine = qmlEngine(this);
        args.append(toJSValue(value, engine));

        if (m_set{{property.name}}.isCallable())
        {
      	  m_set{{property.name}}.call(args);
        }
        else
        {
            qFatal("set{{property.name}} method is not set or not callable");
        }

    }
    {% endif %}

    {% endfor %}

    {{interface}}QMLImplementation() {
        retrieveFrontend();
    }

    {{interface}}QMLImplementationFrontend& interface() const {
        return *m_interface;
    }

    {% for event in interface.signals %}
    Q_INVOKABLE void {{event}}(
            {% set comma = joiner(",") %}
            {% for parameter in event.parameters %}
                {{ comma() }}
                {{parameter|returnType}} {{parameter.name}}
                {% endfor %}
    ) {
        emit m_interface->{{event.name}}(
        {% set comma = joiner(",") %}
        {% for parameter in event.parameters %}
            {{ comma() }}
            {{parameter.name}}
            {% endfor %}
        );
    }
    {% endfor %}

};

inline {{interface}}QMLImplementationFrontend::{{interface}}QMLImplementationFrontend({{interface}}QMLImplementation* qmlImpl) : {{interface}}PropertyAdapter(qmlImpl) {
    m_impl = qmlImpl;
}

inline {{interface}}QMLImplementationFrontend::{{interface}}QMLImplementationFrontend() {
    m_impl = {{interface}}QMLImplementation::createComponent<{{interface}}QMLImplementation>(qmlEngine(), this);
}

inline QObject* {{interface}}QMLImplementationFrontend::impl() {
    return m_impl;
}

{% for operation in interface.operations %}
inline {{operation|returnType}} {{interface}}QMLImplementationFrontend::{{operation.name}}(
    {% set comma = joiner(",") %}
    {% for parameter in operation.parameters %}
    {{ comma() }}
    {{parameter|returnType}} {{parameter.name}}
    {% endfor %}
) {
    return m_impl->{{operation.name}}(
            {% set comma = joiner(",") %}
            {% for parameter in operation.parameters %}
            {{ comma() }}
            {{parameter.name}}
            {% endfor %}
    );
}
{% endfor %}

{% for property in interface.properties %}
	{% if (not property.readonly) %}
inline void {{interface}}QMLImplementationFrontend::set{{property}}(const {{property|returnType}}& newValue) {
	m_impl->requestSet{{property}}(newValue);
}
    {% endif %}
{% endfor %}


{{module|namespaceClose}}
