/**********************************************************************
**
** Copyright (C) 2019 Luxoft Sweden AB
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

#include "InterfaceManager.h"
#include "NewIPCServiceAdapterBase.h"
#include "ipc-common.h"

namespace facelift {

void InterfaceManager::registerAdapter(const QString &objectPath, NewIPCServiceAdapterBase *adapter)
{
    Q_ASSERT(adapter);
    if ((!m_registry.contains(objectPath)) || (m_registry[objectPath] == nullptr)) {
        m_registry.insert(objectPath, adapter);
        qCDebug(LogIpc) << "IPC service registered" << adapter;
    } else {
        qFatal("Can't register new object at path: '%s'. Previously registered object: %s", qPrintable(objectPath),
                qPrintable(facelift::toString(m_registry[objectPath]->service())));
    }
}

void InterfaceManager::unregisterAdapter(NewIPCServiceAdapterBase *adapter) {
    for (auto& key : m_registry.keys()) {
        if (m_registry[key] == adapter) {
            m_registry.remove(key);
            emit adapterUnavailable(key, adapter);
            qCDebug(LogIpc) << "IPC service unregistered" << adapter;
            break;
        }
    }
}

InterfaceBase * InterfaceManager::serviceMatches(const QString& objectPath, NewIPCServiceAdapterBase *adapter) {
    if (adapter->objectPath() == objectPath) {
        return adapter->service();
    } else {
        return nullptr;
    }
}

NewIPCServiceAdapterBase *InterfaceManager::getAdapter(const QString &objectPath)
{
    if (m_registry.contains(objectPath)) {
        return m_registry[objectPath];
    } else {
        return nullptr;
    }
}

InterfaceManager &InterfaceManager::instance()
{
    static auto manager = new InterfaceManager(); // TODO : fix memory leak
    return *manager;
}


}
