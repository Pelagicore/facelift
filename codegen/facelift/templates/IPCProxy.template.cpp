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

{% set className = interfaceName + "IPCProxy" %}

{{module.namespaceCppOpen}}

{{className}}::{{className}}(QObject *parent) : BaseType(parent),
    m_proxies {
    {% if generateAsyncProxy %}
    createIPCAdapter(m_ipcLocalProxyAdapter),
    {% endif %}
#ifdef DBUS_IPC_ENABLED
    createIPCAdapter(m_ipcDBusProxyAdapter),
#endif
    }
{
    ipc()->setObjectPath(SINGLETON_OBJECT_PATH);

    {% if generateAsyncProxy %}
    ipc()->setSynchronous(false);
    {% endif %}

    setIPCProxies(m_proxies);
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
