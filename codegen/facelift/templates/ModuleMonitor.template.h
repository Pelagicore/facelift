/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include "ServiceMonitor.h"

{% for interface in module.interfaces %}
#include "{{interface|fullyQualifiedPath}}Monitor.h"
{% endfor %}

{{module|namespaceOpen}}

class ModuleMonitor : public facelift::ModuleMonitorBase {

public:
    static void registerTypes();

};

{{module|namespaceClose}}

