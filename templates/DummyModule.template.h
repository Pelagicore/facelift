
{% set class = '{0}Module'.format(module.module_name) %}
/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include <QString>
#include <QQmlEngine>

#include "QMLFrontend.h"

{% for interface in module.interfaces %}
#include "{{interface}}Dummy.h"
#include "{{interface|fullyQualifiedPath}}QMLFrontend.h"
{% endfor %}

#include <private/qqmlmetatype_p.h>

{{module|namespaceOpen}}

class {{class}}Dummy {

public:

    static void registerQmlTypes(const char* uri)
    {
        Q_UNUSED(uri);

        {% for interface in module.interfaces %}
        {
            registerQmlComponent<{{interface|fullyQualifiedCppName}}Dummy>(uri, "{{interface.name}}Dummy");

            QString fullyQualifiedTypeName = uri;
            fullyQualifiedTypeName += "/{{interface.name}}";

            // We register the dummy under the "real" name only if nothing is yet registered
            if (QQmlMetaType::qmlType(fullyQualifiedTypeName, 1, 0) == nullptr) {
                qDebug() << "Registering dummy type for interface " << fullyQualifiedTypeName;
                registerQmlComponent<{{interface|fullyQualifiedCppName}}Dummy>(uri, "{{interface.name}}");
            } else {
                qDebug() << "QML type already registered : " << fullyQualifiedTypeName << " => not registering dummy implementation";
            }


        }
        {% endfor %}
    }

};

{{module|namespaceClose}}

