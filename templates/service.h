{% set class = '{0}'.format(interface) %}
{% set comma = joiner(",") %}

/****************************************************************************

** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include <QtCore>

#include "{{module|upperfirst}}Module.h"
#include "model/Model.h"

// Dependencies
{% for property in interface.properties %}
{{property|requiredInclude}}
{% endfor %}

{% for operation in interface.operations %}
{% for parameter in operation.parameters %}
{{parameter|requiredInclude}}
{% endfor %}
{% endfor %}

{% for event in interface.signals %}
{% for parameter in event.parameters %}
{{parameter|requiredInclude}}
{% endfor %}
{% endfor %}


{{module|namespaceOpen}}

class {{class}}QMLFrontend;

/**
 * Definition of the {{interface}} interface.interface
 */
class {{class}} : public ModelInterface {

    Q_OBJECT

public:

    static constexpr const char* INTERFACE_NAME = "{{interface}}";
    static constexpr const char* IPC_INTERFACE_NAME = "{{interface|fullyQualifiedName|lower}}";
    static constexpr const char* IPC_SINGLETON_OBJECT_PATH = "/{{interface|fullyQualifiedName|lower|replace(".","/")}}";

    typedef {{class}}QMLFrontend QMLFrontendType;

    {{class}}(QObject* parent = nullptr) :
            ModelInterface(parent) {
    }

    static void registerTypes(const char* uri) {
        Q_UNUSED( uri);
    }

    {% for property in interface.properties %}

    {% if property.type.is_model -%}

    virtual ModelListModel& {{property}}() = 0;

    {% elif property.type.is_list -%}

    virtual {{property|returnType}} {{property}}() const = 0;

    {% else %}

    virtual {{property|returnType}} {{property}}() const = 0;

    PropertyInterface<{{class}}, {{property|returnType}}> {{property}}Property() { return PropertyInterface<{{class}}, {{property|returnType}}>(this, &{{class}}::{{property}}, &{{class}}::{{property}}Changed); };

    {% if (not property.readonly) %}
    virtual void set{{property}}(const {{property|returnType}}& newValue) = 0;
    {% endif %}

    {% endif %}

    Q_SIGNAL void {{property}}Changed();

    {% endfor %}


    {% for operation in interface.operations %}
    virtual {{operation|returnType}} {{operation}}(
        {% set comma = joiner(",") %}
        {% for parameter in operation.parameters %}
        {{ comma() }}
        {{parameter|returnType}} {{parameter.name}}
        {% endfor %}
    ) = 0;
    {% endfor %}


    {% for event in interface.signals %}
    Q_SIGNAL void {{event}}(
        {% set comma = joiner(",") %}
        {% for parameter in event.parameters %}
        {{ comma() }}
        {{parameter|returnType}} {{parameter.name}}
        {% endfor %}
    );
    {% endfor %}

};


{{module|namespaceClose}}

