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

{{className}}::{{className}}(QObject *parent) : BaseType(parent)
{
    ipc()->setObjectPath(SINGLETON_OBJECT_PATH);
    {% if generateAsyncProxy %}
    ipc()->setSynchronous(false);
    {% endif %}
}

facelift::IPCProxyBinderBase *{{className}}QMLFrontendType::ipc()
{
    auto p = static_cast<{{className}}*>(providerPrivate());
    return p->ipc();
}

{{className}}QMLFrontendType::{{className}}QMLFrontendType(QObject *parent) : {{interfaceName}}QMLFrontend(parent)
{
}

{{className}}QMLFrontendType::{{className}}QMLFrontendType(QQmlEngine *engine) : {{interfaceName}}QMLFrontend(engine)
{
}

{{module.namespaceCppClose}}
