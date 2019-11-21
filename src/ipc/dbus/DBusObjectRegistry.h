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

#pragma once

#include "facelift/ipc/dbus/ObjectRegistryImplementationBase.h"
#include "facelift/ipc/dbus/ObjectRegistryIPCDBusProxy.h"
#include "facelift/ipc/dbus/ObjectRegistryAsyncIPCProxy.h"
#include "facelift/ipc/dbus/ObjectRegistryIPCDBusAdapter.h"


namespace facelift {
namespace dbus {

class FaceliftIPCLibDBus_EXPORT DBusObjectRegistry : public QObject
{
    Q_OBJECT
    static constexpr const char *DEFAULT_SERVICE_NAME = "facelift.registry";
public:

    QString m_serviceName;

    using Content = QMap<QString, QString>;

    class MasterImpl : public facelift::ipc::dbus::ObjectRegistryImplementationBase
    {

    public:
        MasterImpl(DBusObjectRegistry &parent) : facelift::ipc::dbus::ObjectRegistryImplementationBase(&parent)
        {
        }

        void init();

        bool registerObject(const QString &objectPath, const QString &serviceName);

        bool unregisterObject(const QString &objectPath, const QString &serviceName);

    private:
        facelift::ipc::dbus::ObjectRegistryIPCDBusAdapter m_objectRegistryAdapter;

    };

    DBusObjectRegistry(DBusManager &dbusManager);

    void init();

    void registerObject(const QString &objectPath, facelift::AsyncAnswer<bool> answer);

    void unregisterObject(const QString &objectPath);

    const Content &objects(bool blocking);

    Q_SIGNAL void objectsChanged();

private:
    facelift::ipc::dbus::ObjectRegistryIPCDBusProxy *m_objectRegistryProxy = nullptr;
    facelift::ipc::dbus::ObjectRegistryAsyncIPCDBusProxy *m_objectRegistryAsyncProxy = nullptr;
    DBusManager &m_dbusManager;
    bool m_initialized = false;
    std::unique_ptr<MasterImpl> m_master;
};

}
}
