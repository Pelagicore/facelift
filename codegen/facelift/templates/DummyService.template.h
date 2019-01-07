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

{% set class = '{0}'.format(interface) %}

/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

{{classExportDefines}}

#include "{{interface.fullyQualifiedPath}}ImplementationBase.h"
#include "DummyModel.h"

{{module.namespaceCppOpen}}

/**
 * Dummy implementation of the {{class}} API
 */
class {{classExport}} {{class}}Dummy : public {{class}}ImplementationBase {

    Q_OBJECT

public:
    {{class}}Dummy(QObject* parent = nullptr): {{class}}ImplementationBase(parent), m_dummy(*this) {
        setImplementationID("Dummy");
    }

    {% for operation in interface.operations %}

    {% if operation.isAsync %}
    void {{operation}}({% for parameter in operation.parameters %} {{parameter.cppMethodArgumentType}} {{parameter.name}}, {% endfor %}facelift::AsyncAnswer<{{operation.interfaceCppType}}> answer){% if operation.is_const %} const{% endif %} override {
        {% for parameter in operation.parameters %}
        Q_UNUSED({{parameter.name}});
        {% endfor %}
        {% if (operation.hasReturnValue) %}
        {{operation.interfaceCppType}} v = {};
        answer(v);
        {% else %}
        answer();
        {% endif %}
    }
    {% else %}

    {{operation.interfaceCppType}} {{operation}}(
            {% set comma = joiner(",") %}
            {% for parameter in operation.parameters %}
            {{ comma() }}
            {{parameter.cppMethodArgumentType}} {{parameter.name}}
            {% endfor %}
){% if operation.is_const %} const{% endif %} override {

        m_dummy.logMethodCall("{{operation}}",
                { {
                    {% for parameter in operation.parameters %}
                    "{{parameter}}",
                    {% endfor %}
                } }
                {% for parameter in operation.parameters %}
                 , {{parameter.name}}
                {% endfor %}
        );

        {% if (operation.hasReturnValue) %}
        {{operation.interfaceCppType}} returnValue = {};
        return returnValue;
        {% endif %}

    }
    {% endif %}
    {% endfor %}

    {% for property in interface.properties %}
        {% if (not property.readonly) %}
    void set{{property}}({{property.cppMethodArgumentType}} newValue) override {
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
        Dummy({{class}}ImplementationBase& adapter) : facelift::DummyModel<{{class}}>(&adapter), m_adapter(adapter) {
            init();
            {% for property in interface.properties %}
            {% if property.type.is_interface -%}
            // TODO : support interface property type
            {% else %}
            addPropertyWidget(m_adapter.m_{{property.name}}, "{{property.name}}");
            {% endif %}
            {% endfor %}

            {% for event in interface.signals %}
            addSignalWidget("{{event.name}}", { {
                        {% for parameter in event.parameters %}
                        "{{parameter}}",
                        {% endfor %}
                } }, &adapter, &{{class}}::{{event.name}});

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

        {{class}}ImplementationBase& m_adapter;

    };

    private:
        Dummy m_dummy;

};

{{module.namespaceCppClose}}

