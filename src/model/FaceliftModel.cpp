/**********************************************************************
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
**********************************************************************/

#include "FaceliftModel.h"

namespace facelift {

ModelElementID StructureBase::s_nextID = 0;
constexpr int StructureBase::ROLE_ID;

ServiceRegistry::~ServiceRegistry()
{
}

void ServiceRegistry::registerObject(InterfaceBase *i)
{
    m_objects.append(i);

    // Notify later since our object is not yet fully constructed at this point in time
    QTimer::singleShot(0, [this, i] () {
        emit objectRegistered(i);
    });
}

ServiceRegistry &ServiceRegistry::instance()
{
    static ServiceRegistry reg;
    return reg;
}

void ModuleBase::registerQmlTypes(const char *uri, int majorVersion, int minorVersion)
{
    qmlRegisterUncreatableType<facelift::StructureBase>(uri, majorVersion, minorVersion, "StructureBase", "");
    qmlRegisterUncreatableType<facelift::InterfaceBase>(uri, majorVersion, minorVersion, "InterfaceBase", "");
}

}
