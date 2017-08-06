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

QObject* Module_singletontype_provider(QQmlEngine*, QJSEngine*)
{
      return new Module();
}

Module::Module()
    : QObject()
{
}

{% for struct in module.structs %}
{{struct|fullyQualifiedCppName}} Module::create{{struct}}()
{
    return {{struct|fullyQualifiedCppName}}();
}
{% endfor %}

void Module::registerTypes()
{
    {% for enum in module.enums %}
    facelift::qRegisterMetaType<{{enum|fullyQualifiedCppName}}>();
    {% endfor %}

    {% for struct in module.structs %}
    qRegisterMetaType<{{struct|fullyQualifiedCppName}}>();
    {% endfor %}
}

void Module::registerQmlTypes(const char* uri, int majorVersion, int minorVersion)
{
    Q_UNUSED(uri);

    {% for struct in module.structs %}
    ::qmlRegisterType<{{struct}}QMLWrapper>(uri, majorVersion, minorVersion, "{{struct}}");
    {% endfor %}

    qmlRegisterSingletonType<Module>(uri, majorVersion, minorVersion, "Module", Module_singletontype_provider);

    {% for enum in module.enums %}
    qmlRegisterUncreatableType<{{enum|fullyQualifiedCppName}}Gadget>(uri, majorVersion, minorVersion, "{{enum}}", "");
    {% endfor %}

    {% for interface in module.interfaces %}
    {
    	facelift::registerQmlComponent<{{interface}}QMLImplementationFrontend>(uri, "{{interface.name}}QML");

        QString path = STRINGIFY(QML_MODEL_LOCATION) "/{{interface|fullyQualifiedPath}}.qml";

        if (QFile::exists(path)) {
            qDebug() << "Registering QML service implementation : " << path;
            {{interface}}QMLImplementation::modelImplementationFilePath() = path;
            facelift::registerQmlComponent<{{interface}}QMLImplementationFrontend>(uri);
        }
    }
    {% endfor %}

}

{{module|namespaceClose}}
