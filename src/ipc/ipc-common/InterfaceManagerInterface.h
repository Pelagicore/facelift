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
#pragma once

#include <QObject>
#include <QString>
#include <QPointer>
#include "Registry.h"

namespace facelift {

class NewIPCServiceAdapterBase;

class InterfaceManagerInterface : public QObject
{
    Q_OBJECT
public:
    virtual void registerAdapter(const QString &objectPath, NewIPCServiceAdapterBase *adapter) = 0;
    virtual void unregisterAdapter(NewIPCServiceAdapterBase *adapter) = 0;
    virtual NewIPCServiceAdapterBase *getAdapter(const QString &objectPath) = 0;
    virtual Registry<QPointer<NewIPCServiceAdapterBase>>& content() = 0;
    Q_SIGNAL void adapterUnavailable(QString objectPath, NewIPCServiceAdapterBase *adapter);
};

} // end namespace facelift
