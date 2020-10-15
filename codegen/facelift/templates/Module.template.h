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

#include "ModuleBase.h"

{% for interface in module.interfaces %}
#include "{{interface.fullyQualifiedPath}}QMLAdapter.h"
{% endfor %}

{{module.namespaceCppOpen}}

/**
* \brief {{module.name}} module singleton object
* \ingroup {{module.name|toValidId}}
* \inqmlmodule {{module.name}}
* This singleton can be used to create instances of the types defined in the {{module.name}} module/package.
*/
class Module : public facelift::ModuleBase
{
public:
    Module();

    /**
     * Register all QML types provided by this module
     */
    static void registerQmlTypes(const char* uri, int majorVersion = {{module.majorVersion}}, int minorVersion = {{module.minorVersion}});

    /**
     * Register all interfaces as uncreatable types.
     * An interface called "MyInterface" in the QFace file is registered as uncreatable under the name "MyInterface"
     */
    static void registerUncreatableQmlTypes(const char* uri, int majorVersion = {{module.majorVersion}}, int minorVersion = {{module.minorVersion}});

private:
    /**
     * Register metatypes of all types defined by this module. This method should not be called by any hand-written code
     */
    static void registerTypes();
};

{{module.namespaceCppClose}}
