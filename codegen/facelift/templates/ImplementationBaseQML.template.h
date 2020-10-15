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

#include "QMLModel.h"

#include "{{interface}}ImplementationBase.h"
#include "{{interface}}QMLAdapter.h"
{% for operation in interface.operations %}
{% if operation.type.is_interface %}
#include "{{operation.type}}ImplementationBaseQML.h"
{% endif %}
{% endfor %}

{% for struct in interface.referencedStructureTypes %}
{% if struct.reference.isQObjectWrapperEnabled %}
#include "{{struct.fullyQualifiedPath}}QObjectWrapper.h"
{% endif %}
{% endfor %}

{{module.namespaceCppOpen}}

class {{interface}}ImplementationBaseQML;

/**
 * This class implements the actual service interface and wraps the object instantiated from QML, which implements
 * the actual logic
 */
class {{interface}}QMLImplementationProvider : public {{interface}}ImplementationBase,
                                               public facelift::QMLModelImplementationFrontend<{{interface}}ImplementationBaseQML>
{
    Q_OBJECT

public:
    using QMLImplementationModelType = {{interface}}ImplementationBaseQML;
    using AdapterType = {{interface}}ImplementationBase;

    {{interface}}QMLImplementationProvider();
    {{interface}}QMLImplementationProvider({{interface}}ImplementationBaseQML *qmlImpl);

    {% for operation in interface.operations %}
    {% if operation.isAsync %}
    void {{operation}}(
        {%- for parameter in operation.parameters -%}{{parameter.cppMethodArgumentType}} /*{{parameter.name}}*/, {% endfor %}facelift::AsyncAnswer<{{operation.interfaceCppType}}> /*answer*/){% if operation.is_const %} const{% endif %} override
    {
        Q_ASSERT(false);  // TODO: implement
    }
    {% else %}
    {{operation.interfaceCppType}} {{operation.name}}(
    {%- set comma = joiner(", ") -%}
        {%- for parameter in operation.parameters -%}
        {{ comma() }}{{parameter.cppMethodArgumentType}} {{parameter.name}}
        {%- endfor -%}
    ){% if operation.is_const %} const{% endif %} override;
    {% endif %}
    {% endfor %}

    {% for property in interface.properties %}
        {% if (not property.readonly) %}
    void set{{property}}({{property.cppMethodArgumentType}} newValue) override;
        {% endif %}
    {% endfor %}

    static void registerTypes(const char *uri)
    {
        facelift::ModelQMLImplementation<{{interface}}QMLImplementationProvider>::registerTypes(uri);
    }

    QObject* impl();

    friend class {{interface}}ImplementationBaseQML;
};

/**
 * This class defines the QML component which is used when implementing a model using QML
 */
class {{interface}}ImplementationBaseQML : public facelift::ModelQMLImplementation<{{interface}}QMLImplementationProvider>
{
    Q_OBJECT

    Q_PROPERTY(QObject* provider READ provider CONSTANT)

public:
    using Provider = {{interface}}QMLImplementationProvider;
    using ThisType = {{interface}}ImplementationBaseQML;

    static constexpr const char* QML_NAME = "{{interface}}ImplementationBase";
    static constexpr bool ENABLED = true;

    {{interface}}QMLImplementationProvider* createFrontend() override
    {
        return new {{interface}}QMLImplementationProvider(this);
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

    void setReady(bool ready) override
    {
        interface().setReady(ready);
    }

    {% for operation in interface.operations %}

    // {{operation}} method
    Q_PROPERTY(QJSValue {{operation.name}} READ {{operation.name}}JSFunction WRITE set{{operation.name}}JSFunction)
    void set{{operation.name}}JSFunction(QJSValue v)
    {
        m_{{operation.name}} = v;
    }
    QJSValue {{operation.name}}JSFunction() const
    {
        return m_{{operation.name}};
    }
    {% if operation.isAsync %}
    // TODO: implement
    {% else %}
    {{operation.interfaceCppType}} {{operation.name}}(
        {%- set comma = joiner(", ") -%}
        {%- for parameter in operation.parameters -%}
        {{ comma() }}{{parameter.cppMethodArgumentType}} {{parameter.name}}
        {%- endfor %})
    {
        QJSValueList args;
        auto engine = this->qmlEngine();
        if (engine != nullptr) {
            Q_UNUSED(engine);

            {% if operation.parameters %}
            {% for parameter in operation.parameters %}
            args.append(facelift::toJSValue({{parameter.name}}, engine));
            {% endfor %}

            {% endif %}
            {% if operation.hasReturnValue %}
            {{operation.type.qmlCompatibleType}} returnValue;
            auto jsReturnValue = checkMethod(m_{{operation}}, "{{operation}}").call(args);
            facelift::fromJSValue(returnValue, jsReturnValue, engine);
            {% if operation.cppType == operation.type.qmlCompatibleType %}
            return returnValue;
            {% else %}
            {% if operation.type.is_interface %}
            return &((static_cast<{{operation.cppType}}::ImplementationBaseQMLType*>(returnValue))->interface());
            {% else %}
            return facelift::toProviderCompatibleType<{{operation.cppType}}, {{operation.type.qmlCompatibleType}}>(returnValue);
            {% endif %}
            {% endif %}
            {% else %}
            checkMethod(m_{{operation}}, "{{operation}}").call(args);
            {% endif %}
        } else {
            onInvalidQMLEngine();
        }
        {% if operation.hasReturnValue %}
        return {};
        {% endif %}
    }
    {% endif %}
    QJSValue m_{{operation.name}};

    {% endfor %}

    {% for property in interface.properties %}
    Q_SIGNAL void {{property}}Changed();
    {% endfor %}

    {% for property in interface.properties %}

    // {{property}} property
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
        {% if property.type.reference.isQObjectWrapperEnabled %}
        if (m_{{property.name}}.isSet())
            return QVariant::fromValue(m_{{property.name}}.object());
        else
        {% endif %}
            return QVariant::fromValue(interface().m_{{property.name}}.value());
    }
    void set{{property.name}}(const QVariant& var)
    {
        if (var.canConvert<{{property.cppType}}>()) {
            interface().m_{{property.name}} = facelift::fromVariant<{{property.cppType}}>(var);
            {% if property.type.reference.isQObjectWrapperEnabled %}
            m_{{property.name}}.clear();
            {% endif %}
            emit {{property.name}}Changed();
        } {% if property.type.reference.isQObjectWrapperEnabled %} else if (var.canConvert<{{property.cppType}}QObjectWrapper*>()) {
            auto value = qvariant_cast<{{property.cppType}}QObjectWrapper*>(var);
            if (m_{{property.name}}.object() != value) {
                m_{{property.name}}.reset(value);
                sync{{property.name}}();
                if (m_{{property.name}}.isSet()) {
                    m_{{property.name}}.addConnection(QObject::connect(m_{{property.name}}.object(), &{{property.cppType}}QObjectWrapper::anyFieldChanged, this, &{{interface}}ImplementationBaseQML::sync{{property.name}}));
                }
                emit {{property.name}}Changed();
            }
        } {% endif %}
    }
    {% if property.type.reference.isQObjectWrapperEnabled %}
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
    {% endif %}

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
    bool requestSet{{property.name}}(const {{property.cppType}}& value)
    {
        if (!m_set{{property.name}}.isUndefined()) {
            checkInterface();
            QJSValueList args;

            auto qmlEngine = this->qmlEngine();
            if (qmlEngine != nullptr) {
                {% if (not property.type.is_interface) %}
                args.append(facelift::toJSValue(value, qmlEngine));
                {% else %}
                Q_ASSERT(false); // Writable interface properties are unsupported
                {% endif %}

                checkMethod(m_set{{property.name}}, "set{{property.name}}").call(args);
                return true;
            } else {
                onInvalidQMLEngine();
            }
        }
        return false;
    }
    {% endif %}

    {% endfor %}
    {% for event in interface.signals %}

    Q_INVOKABLE void {{event}}(
    {%- set comma = joiner(", ") -%}
    {%- for parameter in event.parameters -%}
    {{ comma() }}{{parameter.type.qmlCompatibleType}} {{parameter.name}}
    {%- endfor -%} )
    {
        emit m_interface->{{event.name}}(
        {%- set comma = joiner(", ") -%}
        {%- for parameter in event.parameters -%}
        {{ comma() }}
        {%- if parameter.cppType == parameter.type.qmlCompatibleType -%}
        {{parameter.name}}
        {%- else -%}
        facelift::toProviderCompatibleType<{{parameter.cppType}}, {{parameter.type.qmlCompatibleType}}>({{parameter.name}})
        {%- endif -%}
        {%- endfor -%} );
    }
    {% endfor %}


    {{interface}}ImplementationBaseQML()
    {
        retrieveFrontend();
    }

    {{interface}}QMLImplementationProvider& interface() const
    {
        return *m_interface;
    }
};


inline {{interface}}QMLImplementationProvider::{{interface}}QMLImplementationProvider({{interface}}ImplementationBaseQML* qmlImpl)
    : {{interface}}ImplementationBase(qmlImpl)
{
    m_impl = qmlImpl;
}

inline {{interface}}QMLImplementationProvider::{{interface}}QMLImplementationProvider()
{
    m_impl = createComponent<{{interface}}ImplementationBaseQML>(qmlEngine(), this);
}

inline QObject* {{interface}}QMLImplementationProvider::impl()
{
    return m_impl;
}

{% for operation in interface.operations %}

{% if operation.isAsync %}
// TODO
{% else %}
inline {{operation.interfaceCppType}} {{interface}}QMLImplementationProvider::{{operation.name}}(
    {%- set comma = joiner(", ") -%}
    {%- for parameter in operation.parameters -%}
    {{ comma() }}{{parameter.cppMethodArgumentType}} {{parameter.name}}
    {%- endfor %}){% if operation.is_const %} const{% endif %}

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
inline void {{interface}}QMLImplementationProvider::set{{property}}({{property.cppMethodArgumentType}} newValue)
{
    if (!m_impl->requestSet{{property}}(newValue)) {
        AdapterType::set{{property}}(newValue);
    }
}

    {% endif %}
{% endfor %}

{{module.namespaceCppClose}}
