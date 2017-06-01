/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include <QtCore>

#include "common/JSON.h"
#include "model/Model.h"
#include "property/Property.h"
#include "dummy/DummyModelCommon.h"

{{module|namespaceOpen}}

enum class {{enum}} {
    {% set comma = joiner(",") %}
    {%- for member in enum.members -%}
    {{ comma() }}
    {{member.name}} = {{member.value}}
    {%- endfor %}
};

class {{enum}}Qml
{
    Q_GADGET
public:
    enum TheEnum {
        {% set comma = joiner(",") %}
        {%- for member in enum.members -%}
        {{ comma() }}
        {{member.name}} = {{member.value}}
        {%- endfor %}        
    };
    Q_ENUM(TheEnum)

};


class {{enum.name}}ListProperty : public EnumListProperty<{{enum.name}}> {

public:

};

{{module|namespaceClose}}

Q_DECLARE_METATYPE({{enum|fullyQualifiedCppName}})


template<> inline void readJSON(const QJsonValue& json, {{enum|fullyQualifiedCppName}}& value) {
    int i = -1;
    readJSON(json, i);
    value = static_cast<{{enum|fullyQualifiedCppName}}>(i);
}

template<> inline void writeJSON(QJsonValue& json, const {{enum|fullyQualifiedCppName}}& value) {
    int i = static_cast<int>(value);
    writeJSON(json, i);
}

template <> inline QVariant toVariant(const {{enum|fullyQualifiedCppName}}& v) {
    return static_cast<int>(v);
}

template<> inline QList<{{enum|fullyQualifiedCppName}}> validValues<{{enum|fullyQualifiedCppName}}>() {
    QList<{{enum|fullyQualifiedCppName}}> values;
    {% for member in enum.members %}
    values.append({{enum|fullyQualifiedCppName}}::{{member}});
    {% endfor %}
    return values;
}

template <> struct DummyUIDesc<{{enum|fullyQualifiedCppName}}> {
    typedef EnumerationPropertyWidget<{{enum|fullyQualifiedCppName}}> PanelType;
};

template <> inline QString toString(const {{enum|fullyQualifiedCppName}}& v) {
    const char* s = "Invalid";
    switch(v) {
    {% for member in enum.members %}
    case {{enum|fullyQualifiedCppName}}::{{member}}:
        s = "{{member}}";
        break;
    {% endfor %}
        default:
            break;
    }
    return s;
}

inline QTextStream &operator <<(QTextStream &outStream, const {{enum|fullyQualifiedCppName}}& f) {
    outStream << toString(f);
    return outStream;
}

inline QJSValue toJSValue(const {{enum|fullyQualifiedCppName}}& f) {
    return enumToJSValue(f);
}

//inline QList<QVariant> toQMLCompatibleType(const QList<{{enum|fullyQualifiedCppName}}>& list) {
//    return toQListQVariantEnum(list);
//}

