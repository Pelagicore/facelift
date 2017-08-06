
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
        addProperty(provider.{{property}}Property(), "{{property}}");
        {% endfor %}
    }

    private:

};

{{module|namespaceClose}}

