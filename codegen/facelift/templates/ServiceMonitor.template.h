
/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include "{{interface|fullyQualifiedPath}}.h"
#include "ServiceMonitor.h"

{{module|namespaceOpen}}

/**
 * Dummy implementation of the {{class}} API
 */
class {{interface}}Monitor : public facelift::ServiceMonitor<{{interface}}> {

    Q_OBJECT

public:
    {{interface}}Monitor(ProviderType_& provider): facelift::ServiceMonitor<{{interface}}>(provider) {

        {% for property in interface.properties %}
        {% if (property.readonly) %}
        addProperty(provider.{{property}}Property(), "{{property}}");
        {% else %}
        addProperty(provider.{{property}}Property(), "{{property}}", &ProviderType_::set{{property}});
        {% endif %}
        {% endfor %}

        {% for signal in interface.signals %}
        logSignal("{{signal}}", &ProviderType_::{{signal}});
        {% endfor %}

    }

    private:

};

{{module|namespaceClose}}

