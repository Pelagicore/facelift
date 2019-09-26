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

#include "IPCProxyBinderBase.h"

namespace facelift {

IPCProxyBinderBase::IPCProxyBinderBase(InterfaceBase &owner, QObject *parent) : QObject(parent), m_owner(owner)
{
}

void IPCProxyBinderBase::connectToServer()
{
    if (!m_alreadyInitialized) {
        m_alreadyInitialized = true;
        bindToIPC();
    }
}

void IPCProxyBinderBase::checkInit()
{
    if (m_componentCompleted && enabled() && !objectPath().isEmpty()) {
        this->connectToServer();
        emit complete();
    }
}

void IPCProxyBinderBase::onComponentCompleted()
{
    m_componentCompleted = true;
    checkInit();
}

void IPCProxyBinderBase::setServiceName(const QString &name)
{
    m_serviceName = name;
    m_explicitServiceName = true;
    checkInit();
}

} // end namespace facelift
