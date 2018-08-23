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

#include "{{class}}.h"

{{module.namespaceCppOpen}}


/**
 */
class {{classExport}} {{class}}Wrapper : public facelift::ServiceWrapper<{{class}}>  {

    Q_OBJECT

public:

    {{class}}Wrapper(QObject* parent = nullptr) : ServiceWrapper<{{class}}>(parent) {
    }

    {% for property in interface.properties %}

    {% if property.type.is_model -%}

    {{property.nestedType.fullyQualifiedCppType}} {{property.name}}ElementAt(size_t index) override {
        return wrapped()->{{property.name}}ElementAt(index);
    }

    size_t {{property.name}}Size() override {
        return wrapped()->{{property.name}}Size();
    }

    {% elif property.type.is_list -%}

    const {{property.cppType}}& {{property}}() const override {
        return wrapped()->{{property}}();
    }

    {% elif property.type.is_interface -%}

    // Service property
    {{property.cppType}}* {{property}}() override {
        return wrapped()->{{property}}();
    }

    {% else %}

    const {{property.cppType}}& {{property}}() const override {
        return wrapped()->{{property}}();
    }

    {% if (not property.readonly) %}
    void set{{property}}(const {{property.cppType}}& newValue) override {
        return wrapped()->set{{property}}(newValue);
    }
    {% endif %}

    {% endif %}

    {% endfor %}


    {% for operation in interface.operations %}
    {{operation.cppType}} {{operation}}(
        {% set comma = joiner(",") %}
        {% for parameter in operation.parameters %}
        {{ comma() }}
        {{parameter.cppType}} {{parameter.name}}
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


{{module.namespaceCppClose}}

