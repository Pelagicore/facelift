{% set class = '{0}Module'.format(module.module_name) %}
/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

{{module|namespaceOpen}}

{% for interface in module.interfaces %}
class {{interface}};
{% endfor %}

{{module|namespaceClose}}

#include <QObject>

{% for struct in module.structs %}
#include "{{struct|fullyQualifiedPath}}.h"
{% endfor %}

{% for enum in module.enums %}
#include "{{enum|fullyQualifiedPath}}.h"
{% endfor %}

{% for interface in module.interfaces %}
#include "{{interface|fullyQualifiedPath}}.h"
{% endfor %}

{{module|namespaceOpen}}

class {{class}} : public QObject {
    Q_OBJECT
public:
    {{class}}(QObject *parent=0);

{% for struct in module.structs %}
    Q_INVOKABLE {{struct|fullyQualifiedCppName}} create{{struct}}();
{% endfor %}

    static void registerTypes();
    static void registerQmlTypes(const char* uri);
};

{{module|namespaceClose}}
