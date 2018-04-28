#include "ModuleMonitor.h"

{{module|namespaceOpen}}

void ModuleMonitor::registerTypes()
{
	if (isEnabled()) {
		auto & serviceMonitorManager = facelift::ServiceMonitorManager::instance();

        {% if (not module.interfaces) %}
        Q_UNUSED(serviceMonitorManager);
        {% endif %}

		{% for interface in module.interfaces %}
		serviceMonitorManager.registerMonitorType<{{interface}}Monitor>();
		{% endfor %}
	}
}

{{module|namespaceClose}}
