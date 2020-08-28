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

#include "{{interfaceName}}IPCProxy.h"
#include "{{interfaceName}}NotAvailableImpl.h"
#include "InterfaceManager.h"

#ifdef DBUS_IPC_ENABLED
#include "{{module.fullyQualifiedPath}}/{{interfaceName}}IPCDBusProxy.h"
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

{% set className = interfaceName + "IPCProxy" %}

{{module.namespaceCppOpen}}


struct {{className}}::Impl {

    Impl() : m_proxies {
        {% if generateAsyncProxy %}
        createIPCProxy(m_ipcLocalProxyAdapter),
        {% endif %}
    #ifdef DBUS_IPC_ENABLED
        createIPCProxy(m_ipcDBusProxyAdapter),
    #endif
        }
    {
    }

{% if generateAsyncProxy %}
    {{interfaceName}}IPCLocalProxyAdapter m_ipcLocalProxyAdapter;
{% endif %}

#ifdef DBUS_IPC_ENABLED
    {{interfaceName}}IPCDBusProxy m_ipcDBusProxyAdapter;
#endif

    {{interfaceName}}NotAvailableImpl m_notAvailableProvider;

    std::array<ProxyAdapterEntry, 0
    {% if generateAsyncProxy %} + 1
    {% endif %}
#ifdef DBUS_IPC_ENABLED
    + 1
#endif
    > m_proxies = {};
};

{{className}}::{{className}}(QObject *parent) : BaseType(facelift::InterfaceManager::instance(), parent),
    m_impl(std::make_unique<Impl>())
{
#ifdef DBUS_IPC_ENABLED
    {% for type in interface.referencedTypes %}
    {% if (not type.is_primitive) %}
    {% if (not type.is_model) %}
    {% if (not type.is_interface) %}
    qDBusRegisterMetaType<{{type.fullyQualifiedCppType}}>();
    qDBusRegisterMetaType<QMap<QString,{{type.fullyQualifiedCppType}}>>();
    qDBusRegisterMetaType<QList<{{type.fullyQualifiedCppType}}>>();
    {% endif %}
    {% endif %}
    {% endif %}
    {% endfor %}
#endif

    ipc()->setObjectPath(SINGLETON_OBJECT_PATH);

    {% if generateAsyncProxy %}
    ipc()->setSynchronous(false);
    {% endif %}

    setIPCProxies(m_impl->m_proxies, m_impl->m_notAvailableProvider);
}

{{className}}::~{{className}}()
{
    resetIPCProxies();
}

facelift::IPCProxyBinderBase *{{className}}QMLAdapterType::ipc()
{
    auto p = static_cast<{{className}}*>(providerPrivate());
    return p->ipc();
}

{{className}}QMLAdapterType::{{className}}QMLAdapterType(QObject *parent) : {{interfaceName}}QMLAdapter(parent)
{
}

{{className}}QMLAdapterType::{{className}}QMLAdapterType(QQmlEngine *engine) : {{interfaceName}}QMLAdapter(engine)
{
}

{{module.namespaceCppClose}}
