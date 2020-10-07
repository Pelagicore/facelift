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

#include "DBusManager.h"
#include "DBusObjectRegistry.h"

namespace facelift {
namespace dbus {

DBusManager::DBusManager()
{
}

DBusManager &DBusManager::instance()
{
    qDBusRegisterMetaType<QMap<QString,QDBusVariant>>();
    qDBusRegisterMetaType<QMap<QString,QString>>();
    qDBusRegisterMetaType<QMap<QString,int>>();
    qDBusRegisterMetaType<QMap<QString,bool>>();
    qDBusRegisterMetaType<QList<QString>>();
    qDBusRegisterMetaType<QList<int>>();
    qDBusRegisterMetaType<QList<bool>>();

    static auto i = new DBusManager(); // TODO solve memory leak
    return *i;
}

DBusObjectRegistry &DBusManager::objectRegistry()
{
    if (m_objectRegistry == nullptr) {
        m_objectRegistry = new DBusObjectRegistry(*this);
    }

    return *m_objectRegistry;
}

} // end namespace dbus
} // end namespace facelift
