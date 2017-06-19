{% set class = 'Module' %}
/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#include "Module.h"

#include <QtQml>

{% for interface in module.interfaces %}
#include "{{interface|fullyQualifiedPath}}QML.h"
#include "{{interface|fullyQualifiedPath}}QMLFrontend.h"
{% endfor %}


{{module|namespaceOpen}}

QObject* {{class}}_singletontype_provider(QQmlEngine*, QJSEngine*)
{
      return new {{class}}();
}

{{class}}::{{class}}(QObject *parent)
    : QObject(parent)
{
}

{% for struct in module.structs %}
{{struct|fullyQualifiedCppName}} {{class}}::create{{struct}}()
{
    return {{struct|fullyQualifiedCppName}}();
}
{% endfor %}

void {{class}}::registerTypes()
{
    {% for enum in module.enums %}
    qRegisterMetaType<{{enum|fullyQualifiedCppName}}>();
    {% endfor %}

    {% for struct in module.structs %}
    qRegisterMetaType<{{struct|fullyQualifiedCppName}}>();
    {% endfor %}
}

void {{class}}::registerQmlTypes(const char* uri, int majorVersion, int minorVersion)
{
    Q_UNUSED(uri);

    {% for struct in module.structs %}
    {% endfor %}

    qmlRegisterSingletonType<{{class}}>(uri, majorVersion, minorVersion, "{{class}}", {{class}}_singletontype_provider);

    {% for enum in module.enums %}
    qmlRegisterUncreatableType<{{enum|fullyQualifiedCppName}}Qml>(uri, majorVersion, minorVersion, "{{enum}}", "");
    {% endfor %}

    {% for interface in module.interfaces %}
    {
        QString path = STRINGIFY(QML_MODEL_LOCATION) "/{{interface|fullyQualifiedPath}}.qml";

        if (QFile::exists(path)) {
            qDebug() << "Registering QML service implementation : " << path;
            {{interface}}QMLImplementation::modelImplementationFilePath() = path;
            registerQmlComponent<{{interface}}QMLImplementationFrontend>(uri);
        }
    }
    {% endfor %}

}

{{module|namespaceClose}}
