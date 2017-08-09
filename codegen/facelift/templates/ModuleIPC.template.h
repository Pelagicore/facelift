{% set class = 'Module' %}
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

class {{class}}IPC {

public:

    static void registerQmlTypes(const char* uri, int majorVersion = {{module.majorVersion}}, int minorVersion = {{module.minorVersion}})
    {
        Q_UNUSED(uri);

        {% for interface in module.interfaces %}
        facelift::qmlRegisterType<{{interface}}IPCAdapter>(uri, "{{interface}}IPCAdapter");

        facelift::registerQmlComponent<{{interface}}IPCProxy>(uri, "{{interface}}IPCProxy");

        {% endfor %}

        ::qmlRegisterUncreatableType<facelift::IPCProxyBinder>(uri, majorVersion, minorVersion, "IPCProxyBinder", QStringLiteral(""));
    }

};

{{module|namespaceClose}}

