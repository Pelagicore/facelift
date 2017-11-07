{% set class = '{0}'.format(interface) %}
{% set comma = joiner(",") %}

/****************************************************************************

** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include <QtCore>

#include "ServiceWrapper.h"

#include "{{class}}.h"

{{module|namespaceOpen}}


/**
 */
class {{class}}Wrapper : public facelift::ServiceWrapper<{{class}}>  {

    Q_OBJECT

public:

    {{class}}Wrapper(QObject* parent = nullptr) : ServiceWrapper<{{class}}>(parent) {
    }

    {% for property in interface.properties %}

    {% if property.type.is_model -%}

    {{property|nestedType|fullyQualifiedCppName}} {{property.name}}ElementAt(size_t index) override {
    	return wrapped()->{{property.name}}ElementAt(index);
    }

    size_t {{property.name}}Size() override {
    	return wrapped()->{{property.name}}Size();
    }

    {% elif property.type.is_list -%}

    const {{property|returnType}}& {{property}}() const override {
    	return wrapped()->{{property}}();
    }

    {% elif property.type.is_interface -%}

    // Service property
    {{property|returnType}}* {{property}}() override {
    	return wrapped()->{{property}}();
    }

    {% else %}

    const {{property|returnType}}& {{property}}() const override {
    	return wrapped()->{{property}}();
    }

    {% if (not property.readonly) %}
    void set{{property}}(const {{property|returnType}}& newValue) override {
    	return wrapped()->set{{property}}(newValue);
    }
    {% endif %}

    {% endif %}

    {% endfor %}


    {% for operation in interface.operations %}
    {{operation|returnType}} {{operation}}(
        {% set comma = joiner(",") %}
        {% for parameter in operation.parameters %}
        {{ comma() }}
        {{parameter|returnType}} {{parameter.name}}
        {% endfor %}
    ) override {
    	return wrapped()->{{operation}}(
    	        {% set comma = joiner(",") %}
    	        {% for parameter in operation.parameters %}
    	        {{ comma() }}
    	        {{parameter.name}}
    	        {% endfor %}
    	);
    }

    {% endfor %}

    void initConnections({{class}}* wrapped) override {
    	Q_UNUSED(wrapped);

        {% for property in interface.properties %}
        addConnection(QObject::connect(wrapped, &{{class}}::{{property.name}}Changed, this, &{{class}}Wrapper::{{property.name}}Changed));
        {% endfor %}

        {% for signal in interface.signals %}
        addConnection(QObject::connect(wrapped, &{{class}}::{{signal.name}}, this, &{{class}}Wrapper::{{signal.name}}));
        {% endfor %}
    }

};


{{module|namespaceClose}}

