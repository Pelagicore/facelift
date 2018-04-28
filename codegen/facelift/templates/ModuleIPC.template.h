/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include <QtQml>
#include "ipc.h"

{% for interface in module.interfaces %}
#include "{{interface|fullyQualifiedPath}}IPC.h"
{% endfor %}

{{module|namespaceOpen}}

class ModuleIPC
{
public:
    static void registerQmlTypes(const char* uri, int majorVersion = {{module.majorVersion}}, int minorVersion = {{module.minorVersion}})
    {
        {% for interface in module.interfaces %}
        // facelift::qmlRegisterType<{{interface}}IPCAdapter>(uri, "{{interface}}IPCAdapter");
        facelift::IPCAdapterFactoryManager::registerType<{{interface}}IPCAdapter>();
        facelift::registerQmlComponent<{{interface}}IPCProxy>(uri, "{{interface}}IPCProxy");

        {% endfor %}
        qmlRegisterUncreatableType<facelift::IPCProxyBinderBase>(uri, majorVersion, minorVersion, "IPCProxyBinder",
                                                     QStringLiteral("Cannot create objects of type IPCProxyBinder"));
        qmlRegisterUncreatableType<facelift::IPCAttachedPropertyFactory>(uri, majorVersion, minorVersion, "IPC",
                                           QStringLiteral("IPCBroker is only available via attached properties"));
    }
};

{{module|namespaceClose}}
