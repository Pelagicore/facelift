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

class {{struct.name}}QObjectWrapper;

/**
 * {{struct.name}} gadget
 */
class {{struct.name}} : public facelift::TModelStructure<
{% for field in struct.fields %}
    {{ comma() }}
    {{field|returnType}}
{% endfor %}
>
{
    Q_GADGET

public:

	typedef {{struct.name}}QObjectWrapper QObjectWrapperType;

    static const FieldNames FIELD_NAMES;

    static const QString& classID() {
    	static auto id = QStringLiteral("{{struct|fullyQualifiedName}}");
    	return id;
    }

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

    const {{field|returnType}}& {{field.name}}() const {
        return m_{{field.name}};
    }

    void set{{field.name}}({{field|returnType}} value) {
//    	qDebug() << "Setting field {{field.name}} with value:" << value;
        m_{{field.name}} = value;
    }

{% endfor %}

private:
{% for field in struct.fields %}
    {{field|returnType}}& m_{{field}};
{% endfor %}

};


{{module|namespaceClose}}

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

{{module|namespaceOpen}}


/**
 * A QObject wrapper of a {{struct.name}}, which is suitable from instantiation from QML, for example
 */
class {{struct.name}}QObjectWrapper : public facelift::StructQObjectWrapper<{{struct.name}}>
{
    Q_OBJECT

public:

    Q_PROPERTY(QString classID READ classID CONSTANT)

    static const QString& classID() {
    	return {{struct|fullyQualifiedCppName}}::classID();
    }

    {{struct.name}}QObjectWrapper(QObject* parent = nullptr) : StructQObjectWrapper(parent)
    {
    	init();
    }

    {{struct.name}}QObjectWrapper(const {{struct.name}}& value, QObject* parent = nullptr) : StructQObjectWrapper(parent)
    {
    	assignFromGadget(value);
    	init();
    }

    void init() {
    	{% for field in struct.fields %}
        m_{{field.name}}.init("{{field.name}}", this, &{{struct.name}}QObjectWrapper::{{field.name}}Changed);
        QObject::connect(this, &{{struct.name}}QObjectWrapper::{{field.name}}Changed, this, &{{struct.name}}QObjectWrapper::anyFieldChanged);
    	{% endfor %}
    }

{% for field in struct.fields %}

	{% set QmlType=field|returnType %}
	{% if field.type.is_enum %}
		{% set QmlType=QmlType + "Gadget::Type" %}
	{% endif %}

    Q_PROPERTY({{QmlType}} {{field}} READ {{field}} WRITE set{{field}} NOTIFY {{field}}Changed)

    Q_SIGNAL void {{field}}Changed();

    const {{field|returnType}}& {{field.name}}() const {
        return m_{{field.name}}.value();
    }

    void set{{field.name}}({{field|returnType}} value) {
        m_{{field.name}} = value;
    }

    facelift::Property<{{field|returnType}}> m_{{field.name}};

{% endfor %}

    Q_PROPERTY({{struct | fullyQualifiedCppName}} gadget READ gadget)

    {{struct.name}} gadget() const {
    	{{struct.name}} s;
    	{% for field in struct.fields %}
    	s.set{{field.name}}(m_{{field.name}}.value());
    	{% endfor %}
    	s.setId(id());
    	return s;
    }

    void assignFromGadget(const {{struct | fullyQualifiedCppName}}& gadget) {
    	{% for field in struct.fields %}
    	m_{{field.name}} = gadget.{{field.name}}();
    	{% endfor %}
    	m_id = gadget.id();
    }

    Q_PROPERTY(QByteArray serialized READ serialized WRITE setSerialized NOTIFY anyFieldChanged)

    QByteArray serialized() const {
    	return gadget().serialize();
    }

    void setSerialized(const QByteArray& array) {
    	{{struct | fullyQualifiedCppName}} v;
    	v.deserialize(array);
    	assignFromGadget(v);
    }

    Q_SIGNAL void anyFieldChanged();

};


class QMLImplListProperty{{struct}} : public facelift::TQMLImplListProperty<{{struct | fullyQualifiedCppName}}> {

//    Q_OBJECT

	typedef facelift::TQMLImplListProperty<{{struct | fullyQualifiedCppName}}> Base;

public:

/*
    Q_INVOKABLE bool elementExists(int elementId) const {
        for (const auto &element : property().value()) {
            if (element.id() == elementId) {
                return true;
            }
        }
        return false;
    }

    Q_INVOKABLE {{struct | fullyQualifiedCppName}} addElement({{struct | fullyQualifiedCppName}} element) {
    	return Base::addElement(element);
    }

    Q_INVOKABLE {{struct | fullyQualifiedCppName}} removeElementByID(int elementId) {
    	return Base::removeElementByID(elementId);
    }

    Q_INVOKABLE {{struct | fullyQualifiedCppName}} addCloneOf({{struct | fullyQualifiedCppName}}QObjectWrapper* element) {
    	return Base::addElement(element->gadget().clone());
    }

    Q_INVOKABLE {{struct | fullyQualifiedCppName}}QObjectWrapper* elementById(int elementId) const {
        auto element = Base::elementById(elementId);

        if (element != nullptr) {
        	return new {{struct | fullyQualifiedCppName}}QObjectWrapper(*element);  // This instance will owned by the QML engine
        }
        else
        	return nullptr;
    }

    Q_INVOKABLE int elementIndexById(int elementId) const {
        return Base::elementIndexById(elementId);
    }

    Q_INVOKABLE {{struct | fullyQualifiedCppName}} elementAt(int index) const {
        return Base::elementAt(index);
    }
*/
};

{{module|namespaceClose}}

namespace facelift {

template<>
class QMLImplListProperty<{{struct | fullyQualifiedCppName}}> : public {{module|fullyQualifiedCppName}}::QMLImplListProperty{{struct}} {

};

}

