
{% set class = 'Module' %}
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

class {{class}}Dummy : public DummyModuleBase {

public:

    static void registerQmlTypes(const char* uri)
    {
        Q_UNUSED(uri);

        {% for interface in module.interfaces %}
        {
            registerQmlComponent<{{interface|fullyQualifiedCppName}}Dummy>(uri, "{{interface.name}}Dummy");
            registerQmlComponentIfNotAlready<{{interface|fullyQualifiedCppName}}Dummy>(uri);
        }
        {% endfor %}
    }

};

{{module|namespaceClose}}

