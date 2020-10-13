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

{{module.namespaceCppOpen}}

class  {{interfaceName}}IPCCommon
{
public:
    enum class MethodID {
        {% for operation in interface.operations %}
        {{operation.name}},
        {% endfor %}
        {% for property in interface.properties %}
        {% if (not property.readonly) %}
        set{{property.name}},
        {% endif %}
        {% if (property.type.is_model) %}
        {{property.name}},  // model
        {% endif %}
        {% endfor %}
    };

    enum class SignalID {
        invalid = static_cast<int>(facelift::CommonSignalID::firstSpecific),
        {% for signal in interface.signals %}
        {{signal.name}},
        {% endfor %}
        {% for property in interface.properties %}
        {{property.name}},
        {% endfor %}
    };

};

{{module.namespaceCppClose}}
