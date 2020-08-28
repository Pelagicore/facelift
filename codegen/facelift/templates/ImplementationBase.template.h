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
#include "ModelProperty.h"

{{module.namespaceCppOpen}}

/**
 * A partial implementation of the service interface, using the Property helper class
 */
class {{classExport}} {{interfaceName}}ImplementationBase : public {{interfaceName}}
{
    Q_OBJECT

    using ThisType = {{interfaceName}}ImplementationBase;

public:
    {{interfaceName}}ImplementationBase(QObject* parent = nullptr);

    {% for property in interface.properties %}

    {% if property.type.is_model %}
    facelift::Model<{{property.nestedType.interfaceCppType}}>& {{property.name}}() override
    {
        return m_{{property.name}};
    }
    facelift::ModelProperty<{{property.nestedType.interfaceCppType}}> m_{{property.name}};
    {% elif property.type.is_list %}
    const {{property.interfaceCppType}}& {{property}}() const override
    {
        return m_{{property.name}}.value();
    }
    facelift::ListProperty<{{property.nestedType.interfaceCppType}}> m_{{property.name}};
    {% elif property.type.is_interface %}
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
    {% if (not property.readonly) %}
    void set{{property}}({{property.cppMethodArgumentType}} newValue) override
    {
        m_{{property.name}} = newValue;
    }
    {% endif %}
    {% endfor %}

    Q_PROPERTY(bool ready READ ready)
    bool ready() const override
    {
        return m_ready.value();
    }

protected:
    void setReady(bool ready)
    {
        m_ready = ready;
    }

    facelift::Property<bool>& readyProperty() {
        return m_ready;
    }

private:
    facelift::Property<bool> m_ready;

};

{{module.namespaceCppClose}}
