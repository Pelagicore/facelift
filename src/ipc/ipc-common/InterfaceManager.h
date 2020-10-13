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
#pragma once

#include <QObject>
#include <QPointer>
#include "Registry.h"
#include "InterfaceManagerInterface.h"

namespace facelift {

class InterfaceBase;

/**
 * This class maintains a registry of IPC services registered locally, which enables local proxies to get a direct reference to them
 */
class InterfaceManager : public InterfaceManagerInterface
{
    Q_OBJECT

public:
    InterfaceManager(const InterfaceManager&) = delete;
    InterfaceManager(const InterfaceManager&&) = delete;
    InterfaceManager& operator=(const InterfaceManager&) = delete;
    InterfaceManager& operator=(const InterfaceManager&&) = delete;

    void registerAdapter(const QString &objectPath, NewIPCServiceAdapterBase *adapter) override;

    void unregisterAdapter(NewIPCServiceAdapterBase *adapter) override;

    NewIPCServiceAdapterBase *getAdapter(const QString &objectPath) override;

    static InterfaceManager &instance();

    static InterfaceBase * serviceMatches(const QString& objectPath, NewIPCServiceAdapterBase *adapter);

    Registry<QPointer<NewIPCServiceAdapterBase>>& content() override
    {
        return m_registry;
    }

private:
    Registry<QPointer<NewIPCServiceAdapterBase>> m_registry;

    // singleton
    InterfaceManager();
};

}
