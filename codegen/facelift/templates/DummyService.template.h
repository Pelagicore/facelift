
{% set class = '{0}'.format(interface) %}

/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include "{{interface|fullyQualifiedPath}}PropertyAdapter.h"
#include "DummyModel.h"

{{module|namespaceOpen}}

/**
 * Dummy implementation of the {{class}} API
 */
class {{class}}Dummy : public {{class}}PropertyAdapter {

    Q_OBJECT

public:
    {{class}}Dummy(QObject* parent = nullptr): {{class}}PropertyAdapter(parent), m_dummy(*this) {
        setImplementationID("Dummy");
    }

    {% for operation in interface.operations %}
    {{operation|returnType}} {{operation}}(
            {% set comma = joiner(",") %}
            {% for parameter in operation.parameters %}
            {{ comma() }}
            {{parameter|returnType}} {{parameter.name}}
            {% endfor %}
) override {


        m_dummy.logMethodCall("{{operation}}",
        		{
					{% for parameter in operation.parameters %}
					"{{parameter}}",
					{% endfor %}
				}
                {% for parameter in operation.parameters %}
                 , {{parameter.name}}
                {% endfor %}
        );

    	{% if (operation.hasReturnValue) %}
        {{operation|returnType}} returnValue = {};
        return returnValue;
        {% endif %}

    }
    {% endfor %}

    {% for property in interface.properties %}
    	{% if (not property.readonly) %}
    void set{{property}}(const {{property|returnType}}& newValue) override {
    	m_dummy.logSetterCall("{{property}}", newValue);
        {% if (not property.type.is_interface) %}
        m_{{property}} = newValue;
        {% else %}
        Q_ASSERT(false); // Writable interface properties are unsupported
        {% endif %}
    }
        {% endif %}
    {% endfor %}



    class Dummy : public facelift::DummyModel<{{class}}> {

    public:
        Dummy({{class}}PropertyAdapter& adapter) : facelift::DummyModel<{{class}}>(&adapter), m_adapter(adapter) {
            init();
            {% for property in interface.properties %}
			{% if property.type.is_interface -%}
			// TODO : support interface property type
			{% else %}
			addPropertyWidget(m_adapter.m_{{property.name}}, "{{property.name}}");
            {% endif %}
            {% endfor %}

            {% for event in interface.signals %}
            addSignalWidget<
			            {% set comma = joiner(",") %}
			            {% for parameter in event.parameters %}
			            {{ comma() }}
			            {{parameter|returnType}}
			            {% endfor %}
			    >("{{event.name}}", {
			            {% for parameter in event.parameters %}
			            "{{parameter}}",
			            {% endfor %}
			    }, &adapter, &{{class}}::{{event.name}});

            {% endfor %}

            finishInit();
        }

		void savePropertyValues(QJsonObject& jsonObject) const override {
			Q_UNUSED(jsonObject);
			{% for property in interface.properties %}

			{% if property.type.is_interface -%}
			// TODO : support interface property type
			{% else %}
			writeJSONProperty(jsonObject, m_adapter.m_{{property.name}}, "{{property.name}}");
			{% endif %}
			{% endfor %}
		}

		void loadPropertyValues(const QJsonObject& jsonObject) override {
			Q_UNUSED(jsonObject);
			{% for property in interface.properties %}
			{% if property.type.is_interface -%}
			// TODO : support interface property type
			{% else %}
			readJSONProperty(jsonObject, m_adapter.m_{{property.name}}, "{{property.name}}");
			{% endif %}
			{% endfor %}
		}

		{{class}}PropertyAdapter& m_adapter;

    };

    private:
        Dummy m_dummy;

};

{{module|namespaceClose}}

