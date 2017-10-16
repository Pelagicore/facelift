/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include <QObject>

{% for struct in module.structs %}
#include "{{struct|fullyQualifiedPath}}.h"
{% endfor %}

{% for enum in module.enums %}
#include "{{enum|fullyQualifiedPath}}.h"
{% endfor %}

{% for interface in module.interfaces %}
#include "{{interface|fullyQualifiedPath}}.h"
#include "{{interface|fullyQualifiedPath}}QMLFrontend.h"
{% endfor %}

{{module|namespaceOpen}}

class Module : public QObject {
    Q_OBJECT
public:
	Module();

{% for struct in module.structs %}
    Q_INVOKABLE {{struct|fullyQualifiedCppName}} create{{struct}}();
{% endfor %}

    static void registerTypes();
    static void registerQmlTypes(const char* uri, int majorVersion = {{module.majorVersion}}, int minorVersion = {{module.minorVersion}});
};

{{module|namespaceClose}}
