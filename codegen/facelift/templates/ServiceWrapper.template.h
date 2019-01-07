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
{% set comma = joiner(",") %}

/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

{{classExportDefines}}

#include <QtCore>

#include "ServiceWrapper.h"

#include "{{module.fullyQualifiedPath}}/{{interfaceName}}.h"

{{module.namespaceCppOpen}}


/**
 */
class {{classExport}} {{interfaceName}}Wrapper : public facelift::ServiceWrapper<{{interfaceName}}>  {

    Q_OBJECT

    using ThisType = {{interfaceName}}Wrapper;

public:

    {{interfaceName}}Wrapper(QObject* parent = nullptr);

    /////////////// Properties
    {% for property in interface.properties %}

    {% if property.type.is_model %}
    facelift::Model<{{property.nestedType.interfaceCppType}}>& {{property.name}}() final override
    {
        return wrapped()->{{property.name}}();
    }
    {% elif property.type.is_list %}
    const {{property.interfaceCppType}}& {{property}}() const final override
    {
        return wrapped()->{{property.name}}();
    }
    {% elif property.type.is_interface %}
    {{property.interfaceCppType}} {{property}}() final override
    {
        return wrapped()->{{property.name}}();
    }
    {% else %}
    const {{property.interfaceCppType}} &{{property}}() const final override
    {
        return wrapped()->{{property.name}}();
    }
    {% endif %}
    {% if (not property.readonly) %}
    void set{{property}}({{property.cppMethodArgumentType}} newValue) final override
    {
        wrapped()->set{{property.name}}(newValue);
    }
    {% endif %}
    {% endfor %}


    /////////////// Methods
    {% for operation in interface.operations %}

    {% if operation.isAsync %}
    void {{operation.name}}(
        {%- for parameter in operation.parameters -%}{{parameter.cppMethodArgumentType}} {{parameter.name}}, {% endfor %}facelift::AsyncAnswer<{{operation.interfaceCppType}}> answer = facelift::AsyncAnswer<{{operation.interfaceCppType}}>()){% if operation.is_const %} const{% endif %} final override {
            return wrapped()->{{operation.name}}(
                    {%- for parameter in operation.parameters -%}
                    {{parameter.name}},
                    {%- endfor -%} answer);
    }
    {% else %}
    {{operation.interfaceCppType}} {{operation.name}}(
        {%- set comma = joiner(", ") -%}
        {%- for parameter in operation.parameters -%}
        {{ comma() }}{{ parameter.cppMethodArgumentType }} {{ parameter.name}}
        {%- endfor -%}  ){% if operation.is_const %} const{% endif %} final override
    {
        {% set comma = joiner(", ") %}
        return wrapped()->{{operation.name}}(
                {%- for parameter in operation.parameters -%}
                {{ comma() }}{{parameter.name}}
                {%- endfor -%} );
    }
    {% endif %}
    {% endfor %}

private:

    void bind({{interfaceName}}* wrapped, {{interfaceName}}* previouslyWrapped) override;

};


{{module.namespaceCppClose}}

