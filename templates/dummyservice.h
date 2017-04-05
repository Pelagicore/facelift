{% set class = '{0}'.format(interface) %}

/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include "{{interface|fullyQualifiedPath}}PropertyAdapter.h"
#include "model/DummyModel.h"

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

        m_dummy.logMethodCall("{{operation}}"
                {% for parameter in operation.parameters %}

                 , {{parameter.name}}
                {% endfor %}
        );
    }
    {% endfor %}

    class Dummy : public DummyModel<{{class}}> {

    public:
        Dummy({{class}}PropertyAdapter& adapter) : DummyModel<{{class}}>(&adapter), m_adapter(adapter) {
            init();
            {% for property in interface.properties %}
            initWidget(m_adapter.m_{{property.name}}, "{{property.name}}");
            {% endfor %}

        }
    void writeJsonValues(QJsonObject& jsonObject) const override {
        Q_UNUSED(jsonObject);
        {% for property in interface.properties %}
        writeJSONProperty(jsonObject, m_adapter.m_{{property.name}}, "{{property.name}}");
        {% endfor %}
    }


    void loadJsonValues(const QJsonObject& jsonObject) override {
        Q_UNUSED(jsonObject);
        {% for property in interface.properties %}
        readJSONProperty(jsonObject, m_adapter.m_{{property.name}}, "{{property.name}}");
        {% endfor %}
    }

    {{class}}PropertyAdapter& m_adapter;

    };

    private:
        Dummy m_dummy;

};

{{module|namespaceClose}}

