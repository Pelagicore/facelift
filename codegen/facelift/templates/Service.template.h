{% set class = '{0}'.format(interface) %}
{% set comma = joiner(",") %}
{% set hasReadiness = interface|hasReadinessProperty -%}

/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include <QtCore>
#include "FaceliftModel.h"

// Dependencies
{% for property in interface.properties %}
{{property|requiredInclude}}
{% endfor %}

{% for operation in interface.operations %}
{% for parameter in operation.parameters %}
{{parameter|requiredInclude}}
{% endfor %}
{% endfor %}

{% for event in interface.signals %}
{% for parameter in event.parameters %}
{{parameter|requiredInclude}}
{% endfor %}
{% endfor %}

{{module|namespaceOpen}}

class {{class}}QMLFrontend;

{% if hasReadiness %}
class Readiness
{
    Q_GADGET

public:
{% for property in interface.properties %}
{% if property.tags.readiness %}

    Q_PROPERTY(bool {{property.name}} READ {{property.name}})
    bool {{property.name}}()
    {
        return m_{{property.name}} ? *m_{{property.name}} : false;
    }
    bool* m_{{property.name}} = nullptr;
{% endif %}
{% endfor %}
};
{% endif %}

{{interface.comment}}
class {{class}} : public facelift::InterfaceBase {

    Q_OBJECT

public:

    static constexpr const char* FULLY_QUALIFIED_INTERFACE_NAME = "{{interface|fullyQualifiedName|lower}}";
    static constexpr const char* INTERFACE_NAME = "{{interface}}";

    static constexpr const int VERSION_MAJOR = {{module.majorVersion}};
    static constexpr const int VERSION_MINOR = {{module.minorVersion}};

    typedef {{class}}QMLFrontend QMLFrontendType;

    friend QMLFrontendType;

    {{class}}(QObject* parent = nullptr);

    static void registerTypes(const char* uri)
    {
        Q_UNUSED( uri);
    }

    {% for property in interface.properties %}
    {% if property.type.is_model -%}
    virtual facelift::Model<{{property|nestedType|fullyQualifiedCppName}}>& {{property.name}}() = 0;

    typedef bool PropertyType_{{property}};   // TODO : use actual type

    facelift::ModelPropertyInterface<{{class}}, {{property|nestedType|fullyQualifiedCppName}}> {{property}}Property()
    {
        return facelift::ModelPropertyInterface<{{class}}, {{property|nestedType|fullyQualifiedCppName}}>();
    }

    {% elif property.type.is_interface -%}

    // Service property
    virtual {{property|returnType}} {{property}}() = 0;

    typedef bool PropertyType_{{property}};   // TODO : use actual type

    facelift::ServicePropertyInterface<{{class}}, {{property|returnType}}> {{property}}Property()
    {
        return facelift::ServicePropertyInterface<{{class}}, {{property|returnType}}>();
    }

    {% if (not property.readonly) %}
    virtual void set{{property}}(const {{property|returnType}}& newValue) = 0;
    {% endif %}

    {% else %}
    virtual const {{property|returnType}}& {{property}}() const = 0;

    facelift::PropertyInterface<{{class}}, {{property|returnType}}> {{property}}Property()
    {
        return facelift::PropertyInterface<{{class}}, {{property|returnType}}>(this, &{{class}}::{{property}}, &{{class}}::{{property}}Changed);
    }

    typedef {{property|returnType}} PropertyType_{{property}};

    {% if (not property.readonly) %}
    virtual void set{{property}}(const {{property|returnType}}& newValue) = 0;
    {% endif %}

    {% endif %}
    Q_SIGNAL void {{property}}Changed();

    {% endfor %}
    {% for operation in interface.operations %}

    {{operation.comment}}
    virtual {{operation|returnType}} {{operation}}({% set comma = joiner(",") %}
        {% for parameter in operation.parameters %}{{ comma() }}{{parameter|returnType}} {{parameter.name}}{% endfor %}) = 0;
    {% endfor %}
    {% for event in interface.signals %}
    {{event.comment}}
    Q_SIGNAL void {{event}}({% set comma = joiner(",") -%}
        {% for parameter in event.parameters -%}{{ comma() }}{{parameter|returnType}} {{parameter.name}}{% endfor %});
    {% endfor %}


    //private:  TODO : make this field private
    mutable QMLFrontendType* m_qmlFrontend = nullptr;


    {% if hasReadiness %}
    const Readiness &readiness() const
    {
        return m_readiness;
    }
    Q_SIGNAL void readinessChanged();

protected:
    Readiness m_readiness;
    {% endif %}
};


{{module|namespaceClose}}

