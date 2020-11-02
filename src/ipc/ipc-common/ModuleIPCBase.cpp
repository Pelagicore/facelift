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

#include "ModuleIPCBase.h"
#include "ipc-common.h"
#include <QtQml>
#include "NewIPCServiceAdapterBase.h"
#include "IPCAttachedPropertyFactory.h"
#include "IPCProxyBinderBase.h"


namespace facelift {

void ModuleIPCBase::registerQmlTypes(const char* uri, int majorVersion, int minorVersion) {
    qmlRegisterUncreatableType<facelift::IPCProxyBinderBase>(uri, majorVersion, minorVersion, "IPCProxyBinder",
                                                 QStringLiteral("Cannot create objects of type IPCProxyBinder"));
    qmlRegisterUncreatableType<facelift::IPCAttachedPropertyFactory>(uri, majorVersion, minorVersion, "IPC",
                                       QStringLiteral("IPCBroker is only available via attached properties"));
}


}
