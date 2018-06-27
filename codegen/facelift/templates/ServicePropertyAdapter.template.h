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

#include "{{class}}.h"
#include "FaceliftProperty.h"

{{module.namespaceCppOpen}}

/**
 * A partial implementation of the service interface, using the Property helper class
 */
class {{class}}PropertyAdapter : public {{class}} {

public:
    {{class}}PropertyAdapter(QObject* parent = nullptr) : {{class}}(parent)
    {
        {% for property in interface.properties %}
        {% if property.tags.hasReadyFlag %}
        m_{{property.name}}.init(this, &{{class}}::{{property.name}}Changed, &{{class}}::readyFlagsChanged, "{{property.name}}");
        m_readyFlags.m_{{property.name}} = &m_{{property.name}}.isReady();
        {% else %}
        m_{{property.name}}.init(this, &{{class}}::{{property.name}}Changed, "{{property.name}}");
        {% endif %}

        {% endfor %}
    }

    {% for property in interface.properties %}
    {% if property.type.is_model %}
    facelift::Model<{{property.nestedType.fullyQualifiedCppType}}>& {{property.name}}() override
    {
        return m_{{property.name}};
    }

    facelift::ModelProperty<{{property.nestedType.fullyQualifiedCppType}}> m_{{property.name}};

    {% elif property.type.is_list %}

    const {{property.interfaceCppType}}& {{property}}() const override
    {
        return m_{{property.name}}.value();
    }

    facelift::ListProperty<{{property.nestedType.interfaceCppType}}> m_{{property.name}};

    {% elif property.type.is_interface -%}

    // Service property
    {{property.interfaceCppType}} {{property}}() override
    {
        return m_{{property.name}}.value();
    }

    facelift::ServiceProperty<{{property.type.fullyQualifiedCppType}}> m_{{property.name}};

    // TODO

    {% else %}
    const {{property.interfaceCppType}} &{{property}}() const override
    {
        return m_{{property.name}}.value();
    }
    facelift::Property<{{property.interfaceCppType}}> m_{{property.name}};

    {% endif %}
    {% endfor %}
};

{{module.namespaceCppClose}}
