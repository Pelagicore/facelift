/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include <QtCore>

#include "Model.h"

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

{{module|namespaceClose}}

Q_DECLARE_METATYPE({{enum|fullyQualifiedCppName}})


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
