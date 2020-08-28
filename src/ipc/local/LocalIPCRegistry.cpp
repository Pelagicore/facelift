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

#include "LocalIPCRegistry.h"
#include "LocalIPC-serialization.h"
#include "LocalIPCRegistry.h"

namespace facelift {
namespace local {


LocalIPCRegistry::LocalIPCRegistry() : m_registry(this)
{
}

void LocalIPCRegistry::registerAdapter(const QString &objectPath, LocalIPCServiceAdapterBase *adapter)
{
    Q_ASSERT(adapter);
    if ((!m_registry.contains(objectPath)) || (m_registry[objectPath] == nullptr)) {
        m_registry.insert(objectPath, adapter);
        qCDebug(LogIpc) << "Local IPC service registered" << adapter << "under path" << objectPath;
    } else {
        qFatal("Can't register new object at path: '%s'. Previously registered object: %s", qPrintable(objectPath),
                qPrintable(facelift::toString(m_registry[objectPath]->service())));
    }
}

void LocalIPCRegistry::unregisterAdapter(LocalIPCServiceAdapterBase *adapter)
{
    for (auto &key : m_registry.keys()) {
        if (m_registry[key] == adapter) {
            m_registry.remove(key);
            qCDebug(LogIpc) << "IPC service unregistered" << adapter;
            break;
        }
    }
}

InterfaceBase *LocalIPCRegistry::serviceMatches(const QString &objectPath, LocalIPCServiceAdapterBase *adapter)
{
    if (adapter->objectPath() == objectPath) {
        return adapter->service();
    } else {
        return nullptr;
    }
}

LocalIPCServiceAdapterBase *LocalIPCRegistry::getAdapter(const QString &objectPath)
{
    if (m_registry.contains(objectPath)) {
        return m_registry[objectPath];
    } else {
        return nullptr;
    }
}

LocalIPCRegistry &LocalIPCRegistry::instance()
{
    static LocalIPCRegistry registry;
    return registry;
}


}

}
