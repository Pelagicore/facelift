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

{{classExportDefines}}

#include "QMLFrontend.h"
#include "DummyModel.h"

{% for interface in module.interfaces %}
#include "{{interface}}Dummy.h"
#include "{{interface.fullyQualifiedPath}}QMLFrontend.h"
{% endfor %}

{{module.namespaceCppOpen}}

class {{classExport}} ModuleDummy : public facelift::DummyModuleBase {

public:

    static void registerQmlTypes(const char* uri, int majorVersion = {{module.majorVersion}}, int minorVersion = {{module.minorVersion}})
    {
        Q_UNUSED(uri);
        Q_UNUSED(majorVersion);
        Q_UNUSED(minorVersion);

        {% for interface in module.interfaces %}
        facelift::registerQmlComponent<{{interface.fullyQualifiedCppType}}Dummy>(uri, "{{interface.name}}Dummy");
//          registerQmlComponentIfNotAlready<{{interface.fullyQualifiedCppType}}Dummy>(uri);
        {% endfor %}
    }

};

{{module.namespaceCppClose}}

