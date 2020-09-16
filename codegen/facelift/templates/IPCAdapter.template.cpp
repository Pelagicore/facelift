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

#include "{{module.fullyQualifiedPath}}/{{interfaceName}}IPCAdapter.h"
#include "IPCServiceAdapterBase.h"
#include <array>
#include "InterfaceManager.h"

#ifdef DBUS_IPC_ENABLED
#include <QtDBus>
#include "{{module.fullyQualifiedPath}}/{{interfaceName}}IPCDBusAdapter.h"
{% for struct in module.structs %}
#include "{{struct.fullyQualifiedPath}}.h"
{% endfor %}

{% for enum in module.enums %}
#include "{{enum.fullyQualifiedPath}}.h"
{% endfor %}

{% for property in interface.referencedInterfaceTypes %}
#include "{{property.fullyQualifiedPath}}{% if generateAsyncProxy %}Async{% endif %}IPCDBusAdapter.h"
{% endfor %}

#define DBUS_MAXIMUM_SIGNATURE_LENGTH 255

{% for type in interface.referencedTypes %}
{% if (not type.is_primitive) %}
{% if (not type.is_model) %}
{% if (not type.is_interface) %}
{{type.requiredInclude}}
{% endif %}
{% endif %}
{% endif %}
{% endfor %}
#endif

{{module.namespaceCppOpen}}

struct {{interfaceName}}IPCAdapter::Impl {

{% if interface.isAsynchronousIPCEnabled %}
    {{interfaceName}}IPCLocalServiceAdapter m_ipcLocalServiceAdapter;
{% endif %}

#ifdef DBUS_IPC_ENABLED
    {{interfaceName}}IPCDBusAdapter m_ipcDBusServiceAdapter;
#endif

    std::array<facelift::IPCServiceAdapterBase*, 0

    {% if interface.isAsynchronousIPCEnabled %}
    +1
    {% endif %}

#ifdef DBUS_IPC_ENABLED
    +1
#endif
    > m_adapters { {
            {% if interface.isAsynchronousIPCEnabled %}
            &m_ipcLocalServiceAdapter,
            {% endif %}
    #ifdef DBUS_IPC_ENABLED
            &m_ipcDBusServiceAdapter,
    #endif
    } };
};


{{interfaceName}}IPCAdapter::{{interfaceName}}IPCAdapter(QObject* parent) :
    BaseType(facelift::InterfaceManager::instance(), parent)
{
#ifdef DBUS_IPC_ENABLED
    {% for type in interface.referencedTypes %}
    {% if (not type.is_primitive) %}
    {% if (not type.is_model) %}
    {% if (not type.is_interface) %}
    qDBusRegisterMetaType<{{type.fullyQualifiedCppType}}>();
    qDBusRegisterMetaType<QMap<QString,{{type.fullyQualifiedCppType}}>>();
    qDBusRegisterMetaType<QList<{{type.fullyQualifiedCppType}}>>();
    {% if type.is_struct %}
    {{type.fullyQualifiedCppType}}::registerDBusTypes();
    Q_ASSERT_X(strlen(QDBusMetaType::typeToSignature(qMetaTypeId<{{type.fullyQualifiedCppType}}>())) < DBUS_MAXIMUM_SIGNATURE_LENGTH, "Signature overflow",
             "Struct's signature exceeds dbus limit, annonate @serializeOverIPC to switch to byte array stream of the struct over dbus, but yet better rethink your structure!");
    {% endif %}
    {% if type.nested.is_struct %}
    {{type.nested.fullyQualifiedCppType}}::registerDBusTypes();
    Q_ASSERT_X(strlen(QDBusMetaType::typeToSignature(qMetaTypeId<{{type.fullyQualifiedCppType}}>())) < DBUS_MAXIMUM_SIGNATURE_LENGTH, "Signature overflow",
             "Struct's signature exceeds dbus limit, annonate @serializeOverIPC to switch to binary array stream of the struct over dbus, but yet better rethink your structure!");
    {% endif %}
    {% endif %}
    {% endif %}
    {% endif %}
    {% endfor %}
#endif
}

{{interfaceName}}IPCAdapter::~{{interfaceName}}IPCAdapter() {
}

void {{interfaceName}}IPCAdapter::createAdapters() {
    m_impl = std::make_unique<Impl>();
    setServiceAdapters(m_impl->m_adapters);
}

void {{interfaceName}}IPCAdapter::destroyAdapters() {
    m_impl.reset();
}

{{module.namespaceCppClose}}
