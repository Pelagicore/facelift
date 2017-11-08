/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#include "Module.h"

#include <QtQml>

#include <QMLModel.h>

{% for interface in module.interfaces %}
#include "{{interface|fullyQualifiedPath}}QMLImplementation.h"
#include "{{interface|fullyQualifiedPath}}QMLFrontend.h"
{% endfor %}

#ifdef ENABLE_IPC
#include "{{module|fullyQualifiedPath}}/ModuleIPC.h"
#endif

#ifdef ENABLE_DESKTOP_TOOLS
#include "{{module|fullyQualifiedPath}}/ModuleMonitor.h"
#include "{{module|fullyQualifiedPath}}/ModuleDummy.h"
#include "ServiceMonitorQMLComponent.h"
#endif

{{module|namespaceOpen}}

QObject* Module_singletontype_provider(QQmlEngine*, QJSEngine*)
{
    return new Module();
}

Module::Module() : facelift::ModuleBase()
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
#ifndef NDEBUG
    static bool alreadyRegistered = false;
    if (alreadyRegistered)
        qWarning() << "Types from facelift module \"{{module.name}}\" already registered. Do not call registerTypes() explicitly";
    alreadyRegistered = true;
#endif

    {% for enum in module.enums %}
    facelift::qRegisterMetaType<{{enum|fullyQualifiedCppName}}>();
    {% endfor %}
    {% for struct in module.structs %}
    qRegisterMetaType<{{struct|fullyQualifiedCppName}}>();
    {% endfor %}

#ifdef ENABLE_DESKTOP_TOOLS
    ModuleMonitor::registerTypes();
#endif

}

void Module::registerQmlTypes(const char* uri, int majorVersion, int minorVersion)
{
    Q_UNUSED(uri);

    registerTypes();

    qmlRegisterSingletonType<Module>(uri, majorVersion, minorVersion, "Module", Module_singletontype_provider);

    qmlRegisterUncreatableType<facelift::QMLImplListPropertyBase>(uri, majorVersion, minorVersion, "QMLImplListPropertyBase", "");
    ModuleBase::registerQmlTypes(uri, majorVersion, minorVersion);

    {% for struct in module.structs %}
    ::qmlRegisterType<{{struct}}QObjectWrapper>(uri, majorVersion, minorVersion, "{{struct}}");
    {% endfor %}

    {% for enum in module.enums %}
    qmlRegisterUncreatableType<{{enum|fullyQualifiedCppName}}Gadget>(uri, majorVersion, minorVersion, "{{enum}}", "");
    {% endfor %}

    // Register components used to implement an interface from QML
    {% for interface in module.interfaces %}
    ::qmlRegisterType<{{interface}}QMLImplementation>(uri, majorVersion, minorVersion, {{interface}}QMLImplementation::QML_NAME);
    {% endfor %}

/*
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
*/

#ifdef ENABLE_IPC
    ModuleIPC::registerQmlTypes(uri, majorVersion, minorVersion);
#endif

#ifdef ENABLE_DESKTOP_TOOLS
    ModuleDummy::registerQmlTypes(uri, majorVersion, minorVersion);
    facelift::ServiceMonitorQMLComponent::registerQmlTypes(uri, majorVersion, minorVersion);
#endif

}

{{module|namespaceClose}}
