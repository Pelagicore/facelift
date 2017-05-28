/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once



#include "{{interface}}PropertyAdapter.h"

{{module|namespaceOpen}}

class {{interface}}QMLImplementation;

/**
 * This class is exposed to the QMLFrontend class
 */
class {{interface}}QMLImplementationFrontend: public {{interface}}PropertyAdapter, public QMLModelImplementationFrontend<{{interface}}QMLImplementation> {

    Q_OBJECT

public:
    typedef {{interface}}QMLImplementation QMLImplementationModelType;

    {{interface}}QMLImplementationFrontend();

    {% for operation in interface.operations %}
    void {{operation.name}}(
            {% set comma = joiner(",") %}
            {% for parameter in operation.parameters %}
            {{ comma() }}
            {{parameter|returnType}} {{parameter.name}}
            {% endfor %}
    ) override;
    {% endfor %}

    {% for property in interface.properties %}
    	{% if (not property.readonly) %}
    virtual void set{{property}}({{property|returnType}} newValue) {}
        {% endif %}
    {% endfor %}

    static void registerTypes(const char* uri) {
        ModelQMLImplementation<{{interface}}QMLImplementationFrontend>::registerTypes(uri);
    }

    QObject* impl();

};

/**
 * This class defines the QML component which is used when implementing a model using QML
 */
class {{interface}}QMLImplementation: public ModelQMLImplementation<{{interface}}QMLImplementationFrontend> {

    Q_OBJECT

public:

    static constexpr const char* QML_NAME = "{{interface}}Implementation";

    {% for operation in interface.operations %}

    inline void {{operation.name}}(
        {% set comma = joiner(",") %}
        {% for parameter in operation.parameters %}
        {{ comma() }}
        {{parameter|returnType}} {{parameter.name}}
        {% endfor %}
    ) {
        QJSValueList args;

        {% for parameter in operation.parameters %}
        args.append(toJSValue({{parameter.name}}));
        {% endfor %}

        if(m_{{operation}}.isCallable())
        {
            m_{{operation}}.call(args);
        }
        else
        {
            qWarning() << "{{operation}} method is not set or not callable";
        }
    }

    Q_PROPERTY(QJSValue {{operation.name}} READ {{operation.name}}JSFunction WRITE set{{operation.name}}JSFunction)

    void set{{operation.name}}JSFunction(QJSValue v) {
        m_{{operation.name}} = v;
    }

    QJSValue {{operation.name}}JSFunction() const {
        return m_{{operation.name}};
    }

    QJSValue m_{{operation.name}};

    {% endfor %}

    {% for property in interface.properties %}

    {%if property.type.is_list or property.type.is_model -%}
    Q_PROPERTY(QObject* {{property.name}} READ {{property.name}});

    QObject* {{property.name}}() {
        return &interface().m_{{property.name}};
    }

    {% else %}
      Q_PROPERTY({{property|returnType}} {{property.name}} READ {{property.name}} WRITE set{{property.name}});
      {{property|returnType}} {{property.name}}() const {
          checkInterface();
          return interface().m_{{property.name}};
      }

      void set{{property.name}}({{property|returnType}} value) {
          checkInterface();
          interface().m_{{property.name}} = value;
      }

    {% endif %}
    {% endfor %}

    {{interface}}QMLImplementation() {
        retrieveFrontend();
    }

    {{interface}}QMLImplementationFrontend& interface() const {
        return *m_interface;
    }

    {% for event in interface.signals %}
    Q_INVOKABLE void {{event}}(
            {% set comma = joiner(",") %}
            {% for parameter in event.parameters %}
                {{ comma() }}
                {{parameter|returnType}} {{parameter.name}}
                {% endfor %}
    ) {
        emit m_interface->{{event.name}}(
        {% set comma = joiner(",") %}
        {% for parameter in event.parameters %}
            {{ comma() }}
            {{parameter.name}}
            {% endfor %}
        );
    }
    {% endfor %}

};

inline {{interface}}QMLImplementationFrontend::{{interface}}QMLImplementationFrontend() {
    m_impl = {{interface}}QMLImplementation::createComponent<{{interface}}QMLImplementation>(qmlEngine(), this);
}

inline QObject* {{interface}}QMLImplementationFrontend::impl() {
    return m_impl;
}

{% for operation in interface.operations %}
inline void {{interface}}QMLImplementationFrontend::{{operation.name}}(
    {% set comma = joiner(",") %}
    {% for parameter in operation.parameters %}
    {{ comma() }}
    {{parameter|returnType}} {{parameter.name}}
    {% endfor %}
) {
    m_impl->{{operation.name}}(
            {% set comma = joiner(",") %}
            {% for parameter in operation.parameters %}
            {{ comma() }}
            {{parameter.name}}
            {% endfor %}
    );
}
{% endfor %}


{{module|namespaceClose}}

