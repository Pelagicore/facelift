{% set class = 'Module' %}
/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include <QtQml>

{% for interface in module.interfaces %}
#include "{{interface|fullyQualifiedPath}}Monitor.h"
{% endfor %}

{{module|namespaceOpen}}

class {{class}}Monitor {

public:

    static void registerTypes();

};

{{module|namespaceClose}}

