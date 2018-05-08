{#*********************************************************************
**
** Copyright (C) 2018 Luxoft Sweden AB
**
** This file is part of the FaceLift project
**
** Permission is hereby granted, free of charge, to any person
** obtaining a copy of this software and associated documentation files
** (the "Software"), to deal in the Software without restriction,
** including without limitation the rights to use, copy, modify, merge,
** publish, distribute, sublicense, and/or sell copies of the Software,
** and to permit persons to whom the Software is furnished to do so,
** subject to the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
** BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
** ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
** SPDX-License-Identifier: MIT
**
*********************************************************************#}

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

Module::Module() : facelift::ModuleBase()
{
}

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


void Module::registerUncreatableQmlTypes(const char* uri, int majorVersion, int minorVersion)
{
    Q_UNUSED(uri);
    Q_UNUSED(majorVersion);
    Q_UNUSED(minorVersion);

    // register an uncreatable type for every interface, so that this type can be used in QML
    {% for interface in module.interfaces %}
    qmlRegisterUncreatableType<{{interface}}QMLFrontend>(uri, majorVersion, minorVersion, "{{interface}}", "");
    {% endfor %}
}


void Module::registerQmlTypes(const char* uri, int majorVersion, int minorVersion)
{
    registerTypes();

    qmlRegisterUncreatableType<facelift::QMLImplListPropertyBase>(uri, majorVersion, minorVersion, "QMLImplListPropertyBase", "");
    qmlRegisterUncreatableType<facelift::QMLImplMapPropertyBase>(uri, majorVersion, minorVersion, "QMLImplMapPropertyBase", "");
    ModuleBase::registerQmlTypes(uri, majorVersion, minorVersion);

    {% for struct in module.structs %}
    // register structure QObject wrapper
    ::qmlRegisterType<{{struct}}QObjectWrapper>(uri, majorVersion, minorVersion, "{{struct}}");
    // register structure gadget factory
    ::qmlRegisterSingletonType<{{struct}}Factory>(uri, majorVersion, minorVersion, "{{struct}}Factory", facelift::StructureFactoryBase::getter<{{struct}}Factory>);
    {% endfor %}

    // register enumeration gadgets
    {% for enum in module.enums %}
    ::qmlRegisterUncreatableType<{{enum|fullyQualifiedCppName}}Gadget>(uri, majorVersion, minorVersion, "{{enum}}", "");
    {% endfor %}

    // Register components used to implement an interface in QML
    {% for interface in module.interfaces %}
    if ({{interface}}QMLImplementation::ENABLED) {
        ::qmlRegisterType<{{interface}}QMLImplementation>(uri, majorVersion, minorVersion, {{interface}}QMLImplementation::QML_NAME);
    }
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
#endif

}

{{module|namespaceClose}}
