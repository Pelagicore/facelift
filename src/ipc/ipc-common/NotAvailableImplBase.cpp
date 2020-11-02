/**********************************************************************
**
** Copyright (C) 2020 Luxoft Sweden AB
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
#include "NotAvailableImplBase.h"
#include "ipc-common.h"

namespace facelift {

void NotAvailableImplBase::logMethodCall(const InterfaceBase &i, const char *methodName)
{
    qCCritical(LogIpc).nospace().noquote() << "Can not call method \"" << methodName << "(...)\" on IPC proxy for interface \"" << i.interfaceID()
                                           << "\". Make sure that the corresponding server is registered";
}

void NotAvailableImplBase::logSetterCall(const InterfaceBase &i, const char *propertyName)
{
    qCCritical(LogIpc).nospace().noquote() << "Can not call setter of property \"" << propertyName << "\" on IPC proxy for interface \"" << i.interfaceID()
                                           << "\". Make sure that the corresponding server is registered";
}

void NotAvailableImplBase::logGetterCall(const InterfaceBase &i, const char *propertyName)
{
    qCDebug(LogIpc) << "Getter of property" << propertyName << "is called" << i.interfaceID() << &i << i.interfaceID();
}

}
