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

/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

{{classExportDefines}}

#include "{{interfaceName}}.h"
#include "FaceliftProperty.h"

{{module.namespaceCppOpen}}

/**
 * A partial implementation of the service interface, using the Property helper class
 */
class {{classExport}} {{interfaceName}}PropertyAdapter : public {{interfaceName}} {

    using ThisType = {{interfaceName}}PropertyAdapter;

public:
    {{interfaceName}}PropertyAdapter(QObject* parent = nullptr) : {{interfaceName}}(parent), m_ready(true)
    {
        m_ready.init(this, &ThisType::readyChanged, "ready");

        {% for property in interface.properties %}
        {% if property.tags.hasReadyFlag %}
        m_{{property.name}}.init(this, &ThisType::{{property.name}}Changed, &ThisType::readyFlagsChanged, "{{property.name}}");
        m_readyFlags.m_{{property.name}} = &m_{{property.name}}.isReady();
        {% else %}
        m_{{property.name}}.init(this, &ThisType::{{property.name}}Changed, "{{property.name}}");
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

    bool ready() const override
    {
        return m_ready.value();
    }

protected:
    void setReady(bool ready)
    {
        m_ready = ready;
    }

private:
    facelift::Property<bool> m_ready;

};

{{module.namespaceCppClose}}
