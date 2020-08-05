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

#include "IPCAttachedPropertyFactory.h"
#include "IPCAdapterFactoryManager.h"
#include "QMLModel.h"

namespace facelift {


IPCAttachedPropertyFactory::IPCAttachedPropertyFactory(QObject *parent) : QObject(parent)
{
}

InterfaceBase *IPCAttachedPropertyFactory::getProvider(QObject *object)
{
    InterfaceBase *provider = nullptr;

    auto o = qobject_cast<facelift::QMLAdapterBase *>(object);
    if (o == nullptr) {
        auto qmlImpl = qobject_cast<facelift::ModelQMLImplementationBase *>(object);
        if (qmlImpl != nullptr) {
            provider = qmlImpl->interfac();
        }
    } else {
        provider = o->providerPrivate();
    }

    return provider;
}

NewIPCServiceAdapterBase *IPCAttachedPropertyFactory::qmlAttachedProperties(QObject *object)
{
    auto provider = getProvider(object);

    NewIPCServiceAdapterBase *serviceAdapter = nullptr;

    if (provider != nullptr) {
        auto interfaceID = provider->interfaceID();
        auto factory = IPCAdapterFactoryManager::instance().getFactory(interfaceID);

        if (factory != nullptr) {
            serviceAdapter = factory(provider);
            serviceAdapter->setEnabled(false);  // We disable by default to force people to write "IPC.enabled: true"
        } else {
            qFatal("No IPC factory found for interface '%s'. Missing '@ipc-sync' or '@ipc-async' decorators ?", qPrintable(interfaceID));
        }
    } else {
        qFatal("Can't attach IPC to object with bad type: %s", object->metaObject()->className());
    }

    return serviceAdapter;
}

}
