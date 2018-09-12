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

#include "QMLModel.h"

#include "{{interface}}PropertyAdapter.h"
#include "{{interface}}QMLFrontend.h"

{{module.namespaceCppOpen}}

class {{interface}}QMLImplementation;

/**
 * This class implements the actual service interface and wraps the object instantiated from QML, which implements
 * the actual logic
 */
class {{classExport}} {{interface}}QMLImplementationFrontend : public {{interface}}PropertyAdapter,
                                               public facelift::QMLModelImplementationFrontend<{{interface}}QMLImplementation>
{
    Q_OBJECT

public:
    typedef {{interface}}QMLImplementation QMLImplementationModelType;

    {{interface}}QMLImplementationFrontend();
    {{interface}}QMLImplementationFrontend({{interface}}QMLImplementation *qmlImpl);

    {% for operation in interface.operations %}
    {% if operation.isAsync %}
    void {{operation}}(
        {%- for parameter in operation.parameters -%}{{parameter.cppType}} /*{{parameter.name}}*/, {% endfor %}facelift::AsyncAnswer<{{operation.cppType}}> /*answer*/){% if operation.is_const %} const{% endif %} override
    {
        Q_ASSERT(false);  // TODO: implement
    }
    {% else %}

    {{operation.interfaceCppType}} {{operation.name}}(
    {%- set comma = joiner(", ") -%}
        {%- for parameter in operation.parameters -%}
        {{ comma() }}{{parameter.cppType}} {{parameter.name}}
        {%- endfor -%}
    ){% if operation.is_const %} const{% endif %} override;
    {% endif %}

    {% endfor %}

    {% for property in interface.properties %}
        {% if (not property.readonly) %}
    void set{{property}}(const {{property.cppType}}& newValue) override;
        {% endif %}
    {% endfor %}

    static void registerTypes(const char *uri)
    {
        facelift::ModelQMLImplementation<{{interface}}QMLImplementationFrontend>::registerTypes(uri);
    }

    QObject* impl();
};

/**
 * This class defines the QML component which is used when implementing a model using QML
 */
class {{classExport}} {{interface}}QMLImplementation : public facelift::ModelQMLImplementation<{{interface}}QMLImplementationFrontend>
{
    Q_OBJECT

    Q_PROPERTY(QObject* provider READ provider CONSTANT)

public:
    typedef {{interface}}QMLImplementationFrontend Provider;
    typedef {{interface}}QMLImplementation ThisType;

    static constexpr const char* QML_NAME = "{{interface}}QMLImplementation";
    static constexpr bool ENABLED = true;

    {{interface}}QMLImplementationFrontend* createFrontend() override
    {
        return new {{interface}}QMLImplementationFrontend(this);
    }

    void initProvider(Provider *provider) override
    {
        Q_UNUSED(provider);

        {% for property in interface.properties %}
        QObject::connect(provider, &Provider::{{property}}Changed, this, &ThisType::{{property}}Changed);
        {% endfor %}

        {% for property in interface.properties %}
            {% if property.type.is_list or property.type.is_map %}
        m_{{property.name}}QMLProperty.setProperty(interface().m_{{property.name}});
            {% endif %}
        {% endfor %}
    }

    {% for operation in interface.operations %}
    {% if operation.isAsync %}
    // TODO: implement
    {% else %}

    {{operation.interfaceCppType}} {{operation.name}}(
        {%- set comma = joiner(", ") %}
        {% for parameter in operation.parameters %}
        {{ comma() }}{{parameter.cppType}} {{parameter.name}}
        {%- endfor %})
    {
        QJSValueList args;
        QQmlEngine* engine = qmlEngine(this);
        Q_UNUSED(engine);

        {% if operation.parameters %}
        {% for parameter in operation.parameters %}
        args.append(facelift::toJSValue({{parameter.name}}, engine));
        {% endfor %}

        {% endif %}
        {% if operation.hasReturnValue %}
        {{operation.interfaceCppType}} returnValue;
        auto jsReturnValue = checkMethod(m_{{operation}}, "{{operation}}").call(args);
        facelift::fromJSValue(returnValue, jsReturnValue, engine);
        return returnValue;
        {% else %}
        checkMethod(m_{{operation}}, "{{operation}}").call(args);
        {% endif %}
    }

    {% endif %}

    Q_PROPERTY(QJSValue {{operation.name}} READ {{operation.name}}JSFunction WRITE set{{operation.name}}JSFunction)

    void set{{operation.name}}JSFunction(QJSValue v)
    {
        m_{{operation.name}} = v;
    }

    QJSValue {{operation.name}}JSFunction() const
    {
        return m_{{operation.name}};
    }

    QJSValue m_{{operation.name}};

    {% endfor %}

    {% for property in interface.properties %}
    Q_SIGNAL void {{property}}Changed();
    {% endfor %}

    {% for property in interface.properties %}
    {% if property.type.is_list or property.type.is_map %}
    Q_PROPERTY({{property.type.qmlCompatibleType}} {{property.name}} READ {{property.name}} WRITE set{{property.name}} NOTIFY {{property.name}}Changed)

    facelift::QMLImpl{{property.type.name|capitalize}}Property<{{property.nestedType.interfaceCppType}}> m_{{property.name}}QMLProperty;

    {{property.type.qmlCompatibleType}} {{property.name}}() const
    {
        return m_{{property.name}}QMLProperty.elementsAsVariant();
    }

    void set{{property.name}}({{property.type.qmlCompatibleType}} v)
    {
        m_{{property.name}}QMLProperty.setElementsAsVariant(v);
    }

    {% elif property.type.is_model %}
    // TODO : model

    {% elif property.type.is_interface %}
    // TODO : interface

    {% elif property.type.is_struct %}
    // This property can contain either a {{property.cppType}} (gadget), or a {{property.cppType}}QObjectWrapper
    Q_PROPERTY(QVariant {{property.name}} READ {{property.name}} WRITE set{{property.name}} NOTIFY {{property.name}}Changed)

    QVariant {{property.name}}() const
    {
        if (m_{{property.name}}.isSet())
            return QVariant::fromValue(m_{{property.name}}.object());
        else
            return QVariant::fromValue(interface().m_{{property.name}}.value());
    }

    void set{{property.name}}(const QVariant& var)
    {
        if (var.canConvert<{{property.cppType}}>()) {
            interface().m_{{property.name}} = facelift::fromVariant<{{property.cppType}}>(var);
            m_{{property.name}}.clear();
            emit {{property.name}}Changed();
        } else if (var.canConvert<{{property.cppType}}QObjectWrapper*>()) {
            auto value = qvariant_cast<{{property.cppType}}QObjectWrapper*>(var);
            if (m_{{property.name}}.object() != value) {
                m_{{property.name}}.reset(value);
                sync{{property.name}}();
                if (m_{{property.name}}.isSet()) {
                    m_{{property.name}}.addConnection(QObject::connect(m_{{property.name}}.object(), &{{property.cppType}}QObjectWrapper::anyFieldChanged, this, &{{interface}}QMLImplementation::sync{{property.name}}));
                }
                emit {{property.name}}Changed();
            }
        }
    }

    void sync{{property.name}}()
    {
        checkInterface();
        {{property.cppType}} value;
        if (m_{{property.name}}.isSet()) {
            value = m_{{property.name}}.object()->gadget();
            interface().m_{{property.name}} = value;
        }
    }

    facelift::QObjectWrapperPointer<{{property.cppType}}QObjectWrapper> m_{{property.name}};

    {% else %}
    Q_PROPERTY({{property.type.qmlCompatibleType}} {{property.name}} READ {{property.name}} WRITE set{{property.name}} NOTIFY {{property.name}}Changed)

    const {{property.type.qmlCompatibleType}}& {{property.name}}() const
    {
        checkInterface();
        return interface().m_{{property.name}};
    }

    void set{{property.name}}(const {{property.type.qmlCompatibleType}}& value)
    {
        checkInterface();
        interface().m_{{property.name}} = value;
    }

    {% endif %}
    {% if (not property.readonly) %}
    Q_PROPERTY(QJSValue set{{property.name}} READ set{{property.name}}JSFunction WRITE setset{{property.name}}JSFunction)

    void setset{{property.name}}JSFunction(QJSValue v)
    {
        m_set{{property.name}} = v;
    }

    QJSValue set{{property.name}}JSFunction() const
    {
        return m_set{{property.name}};
    }

    QJSValue m_set{{property.name}};

    void requestSet{{property.name}}(const {{property.cppType}}& value)
    {
        checkInterface();
        QJSValueList args;

        QQmlEngine* engine = qmlEngine(this);
        {% if (not property.type.is_interface) %}
        args.append(facelift::toJSValue(value, engine));
        {% else %}
        Q_ASSERT(false); // Writable interface properties are unsupported
        {% endif %}

        checkMethod(m_set{{property.name}}, "set{{property.name}}").call(args);
    }

    {% endif %}
    {% endfor %}

    {{interface}}QMLImplementation()
    {
        retrieveFrontend();
    }

    {{interface}}QMLImplementationFrontend& interface() const
    {
        return *m_interface;
    }

    {% for event in interface.signals %}

    Q_INVOKABLE void {{event}}(
    {%- set comma = joiner(", ") -%}
    {%- for parameter in event.parameters -%}
    {{ comma() }}{{parameter.interfaceCppType}} {{parameter.name}}
    {%- endfor -%} )
    {
        emit m_interface->{{event.name}}(
        {%- set comma = joiner(", ") -%}
        {%- for parameter in event.parameters -%}
        {{ comma() }}{{parameter.name}}
        {%- endfor -%} );
    }
    {% endfor %}
};


inline {{interface}}QMLImplementationFrontend::{{interface}}QMLImplementationFrontend({{interface}}QMLImplementation* qmlImpl)
    : {{interface}}PropertyAdapter(qmlImpl)
{
    m_impl = qmlImpl;
}

inline {{interface}}QMLImplementationFrontend::{{interface}}QMLImplementationFrontend()
{
    m_impl = createComponent<{{interface}}QMLImplementation>(qmlEngine(), this);
}

inline QObject* {{interface}}QMLImplementationFrontend::impl()
{
    return m_impl;
}

{% for operation in interface.operations %}

{% if operation.isAsync %}
// TODO
{% else %}
inline {{operation.interfaceCppType}} {{interface}}QMLImplementationFrontend::{{operation.name}}(
    {%- set comma = joiner(", ") -%}
    {%- for parameter in operation.parameters -%}
    {{ comma() }}{{parameter.cppType}} {{parameter.name}}
    {%- endfor -%} ){% if operation.is_const %} const{% endif %}
{
    return m_impl->{{operation.name}}(
        {%- set comma = joiner(", ") -%}
        {%- for parameter in operation.parameters -%}
        {{ comma() }}{{parameter.name}}
        {%- endfor -%} );
}
{% endif %}

{% endfor %}

{% for property in interface.properties %}
    {% if (not property.readonly) %}
inline void {{interface}}QMLImplementationFrontend::set{{property}}(const {{property.cppType}}& newValue)
{
    m_impl->requestSet{{property}}(newValue);
}

    {% endif %}
{% endfor %}

{{module.namespaceCppClose}}
