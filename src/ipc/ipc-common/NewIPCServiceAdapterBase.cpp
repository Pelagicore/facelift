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

#include "NewIPCServiceAdapterBase.h"
#include "IPCServiceAdapterBase.h"

namespace facelift {

NewIPCServiceAdapterBase::NewIPCServiceAdapterBase(QObject *parent) : QObject(parent)
{
}

NewIPCServiceAdapterBase::~NewIPCServiceAdapterBase() {
    unregisterLocalService();
}

void NewIPCServiceAdapterBase::registerService()
{
    registerLocalService();
    createAdapters();
    for (auto& ipcAdapter : m_ipcServiceAdapters) {
        ipcAdapter->registerService(objectPath(), service());
    }
}

void NewIPCServiceAdapterBase::unregisterService()
{
    unregisterLocalService();
    destroyAdapters();
    m_ipcServiceAdapters.clear();
}

void NewIPCServiceAdapterBase::setServiceAdapters(facelift::span<IPCServiceAdapterBase*> adapters) {
    m_ipcServiceAdapters = adapters;
}

void NewIPCServiceAdapterBase::onValueChanged()
{
    if (isReady()) {
        if (!m_registered) {
            registerService();
            m_registered = true;
        }
    } else {
        if (m_registered) {
            unregisterService();
            m_registered = false;
        }
    }
}

}
