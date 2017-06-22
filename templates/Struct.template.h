/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/
{% set comma = joiner(",") %}

#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <QTextStream>

#include "Model.h"
#include "QMLModel.h"

// Dependencies
{% for field in struct.fields %}
{{field|requiredInclude}}
{% endfor %}

{{module|namespaceOpen}}

class {{struct.name}} : public TModelStructure<
{% for field in struct.fields %}
    {{ comma() }}
    {{field|returnType}}
{% endfor %}
>
{
    Q_GADGET

public:

    static const FieldNames FIELD_NAMES;

    Q_PROPERTY(int id READ id CONSTANT)   // This seems to be necessary even if the base class already contains an "id" property. TODO : clarify

    {{struct.name}}()
	{% if struct.fields %}:{% endif %}
    {%-for field in struct.fields %}
        m_{{field}}(std::get<{{loop.index-1}}>(m_values))
        {% if not loop.last %},{% endif %}
    {% endfor %}
    {
    }

    {{struct.name}}(const {{struct.name}}& other)
    {% if struct.fields %}:{% endif %}
    {%-for field in struct.fields %}
        m_{{field}}(std::get<{{loop.index-1}}>(m_values))
        {% if not loop.last %},{% endif %}
    {% endfor %}
    {
        copyFrom(other);
    }

    {{struct.name}}& operator=(const {{struct.name}} &right) {
        copyFrom(right);
        return *this;
    }

    static QHash<int, QByteArray> roleNames() {
        return roleNames_(FIELD_NAMES);
    }

    Q_INVOKABLE {{struct|fullyQualifiedCppName}} clone() const {
        {{struct.name}} s;
        s.setValue(asTuple());
        return s;
    }

    QString toString() const {
        return toStringWithFields(FIELD_NAMES);
    }

{% for field in struct.fields %}

	{% set QmlType=field|returnType %}
	{% if field.type.is_enum %}
		{% set QmlType=QmlType + "Gadget::Type" %}
	{% endif %}

    Q_PROPERTY({{QmlType}} {{field}} READ {{field}} WRITE set{{field}})

    {{field|returnType}} {{field.name}}() const {
        return m_{{field.name}};
    }

    void set{{field.name}}({{field|returnType}} value) {
//    	qDebug() << "Setting field {{field.name}} with value:"; // << value;
        m_{{field.name}} = value;
    }

{% endfor %}

public:
{% for field in struct.fields %}
    {{field|returnType}}& m_{{field}};
{% endfor %}

};


class QMLImplListProperty{{struct}} : public TQMLImplListProperty<{{struct | fullyQualifiedCppName}}> {

    Q_OBJECT

public:

    Q_INVOKABLE bool elementExists(int elementId) const {
        for (const auto &element : property().value()) {
            if (element.id() == elementId) {
                return true;
            }
        }
        return false;
    }

    Q_INVOKABLE void addElement({{struct | fullyQualifiedCppName}} element) {
    	TQMLImplListProperty::addElement(element);
    }

    Q_INVOKABLE {{struct | fullyQualifiedCppName}} elementById(int elementId) const {
        auto element = TQMLImplListProperty::elementById(elementId);
        Q_ASSERT(element != nullptr);
        return *element;
    }

    Q_INVOKABLE int elementIndexById(int elementId) const {
        return TQMLImplListProperty::elementIndexById(elementId);
    }

    Q_INVOKABLE {{struct | fullyQualifiedCppName}} elementAt(int index) const {
        return TQMLImplListProperty::elementAt(index);
    }

};

{{module|namespaceClose}}



template<>
class QMLImplListProperty<{{struct | fullyQualifiedCppName}}> : public {{module|fullyQualifiedCppName}}::QMLImplListProperty{{struct}} {

};


inline QTextStream &operator <<(QTextStream &outStream, const {{struct|fullyQualifiedCppName}}& f) {
    outStream << f.toString();
    return outStream;
}

inline QDebug operator<< (QDebug d, const {{struct|fullyQualifiedCppName}} &f) {
	QString s;
	QTextStream stream(&s);
	stream << f;
	d << s;
    return d;
}

Q_DECLARE_METATYPE(QList<{{struct|fullyQualifiedCppName}}>)   // Needed for list properties
Q_DECLARE_METATYPE({{struct|fullyQualifiedCppName}})

