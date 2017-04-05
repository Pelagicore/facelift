{% set class = '{0}'.format(interface) %}
/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include "{{module|upperfirst}}Module.h"
#include "{{class}}.h"
#include "property/Property.h"

{{module|namespaceOpen}}

/**
 * Definition of the service interface
 */
class {{class}}PropertyAdapter : public {{class}} {

public:

    {{class}}PropertyAdapter(QObject* parent = nullptr) : {{class}}(parent) {
        {% for property in interface.properties %}
        {%if property.type.is_model -%}
        {% else %}
          m_{{property.name}}.init(this, &{{class}}::{{property.name}}Changed);
        {% endif %}
        {% endfor %}
    }

    {% for property in interface.properties %}

    {%if property.type.is_model %}

    virtual ::ModelListModel& {{property.name}}() override {
        return m_{{property.name}}.getModel();
    }

    {{property|nestedType|fullyQualifiedCppName}}ListProperty m_{{property.name}};

    {%elif property.type.is_list %}

    {{property|returnType}} {{property}}() const override {
        return m_{{property.name}};
    }

    {{property|nestedType|fullyQualifiedCppName}}ListProperty m_{{property.name}};

    {% else %}

    {{property|returnType}} {{property}}() const override {
        return m_{{property.name}};
    }
    ::Property<{{property|returnType}}> m_{{property.name}};
    {% endif %}

    {% endfor %}

};

{{module|namespaceClose}}



