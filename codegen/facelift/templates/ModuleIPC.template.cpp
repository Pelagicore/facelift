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

#include "ModuleIPC.h"
#include "ipc-common/ModuleIPCBase.h"
#include "facelift-ipc.h"

{% for interface in module.interfaces %}

{% if interface.isIPCEnabled %}
#include "{{interface.fullyQualifiedPath}}IPCAdapter.h"
{% endif %}

{% if interface.isSynchronousIPCEnabled %}
#include "{{interface.fullyQualifiedPath}}IPCProxy.h"
{% endif %}

{% if interface.isAsynchronousIPCEnabled %}
#include "{{interface.fullyQualifiedPath}}AsyncIPCProxy.h"
#include "{{interface.fullyQualifiedPath}}AsyncQMLFrontend.h"
{% endif %}

{% endfor %}

{{module.namespaceCppOpen}}

void ModuleIPC::registerQmlTypes(const char* uri, int majorVersion, int minorVersion)
{
    {% for interface in module.interfaces %}

        {% if interface.isIPCEnabled %}
    facelift::qmlRegisterType<{{interface}}IPCAdapter>(uri, "{{interface}}IPCAdapter");
    facelift::IPCAdapterFactoryManager::registerType<{{interface}}IPCAdapter>();
        {% endif %}

        {% if interface.isSynchronousIPCEnabled %}
    facelift::registerQmlComponent<{{interface}}IPCProxy>(uri, "{{interface}}IPCProxy");
        {% endif %}

        {% if interface.isAsynchronousIPCEnabled %}
    facelift::registerQmlComponent<{{interface}}AsyncIPCProxy>(uri, "{{interface}}AsyncIPCProxy");
        {% endif %}

    {% endfor %}

    facelift::ModuleIPCBase::registerQmlTypes(uri, majorVersion, minorVersion);
}

{{module.namespaceCppClose}}
