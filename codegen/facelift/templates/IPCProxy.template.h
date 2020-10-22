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

#pragma once

#include "IPCProxy.h"
#include "NotAvailableImpl.h"
#include "NewIPCServiceAdapterBase.h"

#include "{{module.fullyQualifiedPath}}/{{interfaceName}}ServiceWrapper.h"
#include "{{module.fullyQualifiedPath}}/{{interfaceName}}.h"
#include "{{module.fullyQualifiedPath}}/{{interfaceName}}QMLAdapter.h"

{% if generateAsyncProxy %}
#include "{{module.fullyQualifiedPath}}/{{interfaceName}}IPCLocalProxyAdapter.h"
{% endif %}

{% for property in interface.referencedInterfaceTypes %}
#include "{{property.fullyQualifiedPath}}{% if generateAsyncProxy %}Async{% endif %}IPCProxy.h"
{% endfor %}

{% set className = interfaceName + "IPCProxy" %}

{{module.namespaceCppOpen}}



class {{className}}QMLAdapterType;

class {{className}} : public facelift::IPCProxy<{{interfaceName}}Wrapper>
{
    Q_OBJECT

public:
    using ThisType = {{className}};
    using BaseType = facelift::IPCProxy<{{interfaceName}}Wrapper>;

    // override the default QMLAdapter type to add the IPC related properties
    using QMLAdapterType = {{className}}QMLAdapterType;

    {{className}}(QObject *parent = nullptr);
    ~{{className}}();

private:
    struct Impl;

    std::unique_ptr<Impl> m_impl;
};


class {{className}}QMLAdapterType : public {{interfaceName}}QMLAdapter
{
    Q_OBJECT

    Q_PROPERTY(facelift::IPCProxyBinderBase *ipc READ ipc CONSTANT)

public:
    {{className}}QMLAdapterType(QObject *parent = nullptr);

    {{className}}QMLAdapterType(QQmlEngine *engine);

    facelift::IPCProxyBinderBase *ipc();

};


{{module.namespaceCppClose}}
