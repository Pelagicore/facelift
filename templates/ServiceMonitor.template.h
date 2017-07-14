
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
class {{interface}}Monitor : public ServiceMonitor<{{interface}}> {

    Q_OBJECT

	typedef {{interface}} ProviderType;

public:
    {{interface}}Monitor(ProviderType& provider): ServiceMonitor(provider) {
    }

    private:

};

{{module|namespaceClose}}

