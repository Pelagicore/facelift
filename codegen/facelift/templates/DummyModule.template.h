/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include <QString>
#include <QQmlEngine>

#include "QMLFrontend.h"
#include "DummyModel.h"

{% for interface in module.interfaces %}
#include "{{interface}}Dummy.h"
#include "{{interface|fullyQualifiedPath}}QMLFrontend.h"
{% endfor %}

{{module|namespaceOpen}}

class ModuleDummy : public facelift::DummyModuleBase {

public:

    static void registerQmlTypes(const char* uri, int majorVersion = {{module.majorVersion}}, int minorVersion = {{module.minorVersion}})
    {
        Q_UNUSED(uri);
        Q_UNUSED(majorVersion);
        Q_UNUSED(minorVersion);

        {% for interface in module.interfaces %}
        facelift::registerQmlComponent<{{interface|fullyQualifiedCppName}}Dummy>(uri, "{{interface.name}}Dummy");
//      	registerQmlComponentIfNotAlready<{{interface|fullyQualifiedCppName}}Dummy>(uri);
        {% endfor %}
    }

};

{{module|namespaceClose}}

