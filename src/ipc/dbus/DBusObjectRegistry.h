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
#include "facelift/ipc/dbus/ObjectRegistryAsyncIPCDBusProxy.h"
#include "facelift/ipc/dbus/ObjectRegistryIPCDBusAdapter.h"
#include "Registry.h"
#include "DBusManagerInterface.h"

namespace facelift {
namespace dbus {

class FaceliftIPCLibDBus_EXPORT DBusObjectRegistry : public QObject
{
    Q_OBJECT
    static constexpr const int INVALID_REGISTRY_VERSION = 0;
    static constexpr const int INITIAL_REGISTRY_VERSION = 1;
    static const QString VERSION_KEY;
public:

    QString m_serviceName;

    class MasterImpl : public facelift::ipc::dbus::ObjectRegistryImplementationBase
    {

    public:
        MasterImpl() = default;

        void init();

        bool registerObject(const QString &objectPath, const QString &serviceName) override;

        bool unregisterObject(const QString &objectPath, const QString &serviceName) override;

        QMap<QString, QString> getObjects() override;

    private:
        void updateVersion();

        int m_version = DBusObjectRegistry::INVALID_REGISTRY_VERSION;
        QMap<QString, QString> m_objectMap;
        facelift::ipc::dbus::ObjectRegistryIPCDBusAdapter m_objectRegistryAdapter;
    };

    DBusObjectRegistry(DBusManagerInterface &dbusManager);

    void registerObject(const QString &objectPath, const QString& serviceName, facelift::AsyncAnswer<bool> answer);

    void unregisterObject(const QString &objectPath, const QString& serviceName);

    const Registry<QString>& objects(bool blocking);

private:
    void init();

    static int nextVersion(const int currentVersion);

    void onObjectAdded(const QString& objectPath, const QString& serviceName, int registryVersion);
    void onObjectRemoved(const QString& objectPath, int registryVersion);
    bool isMaster() const;

    void syncObjects();
    void updateObjects(const QMap<QString, QString>& objectMap);
    bool hasValidObjects() const { return m_registryVersion != INVALID_REGISTRY_VERSION; }

    int version() const { return m_registryVersion; }

    facelift::ipc::dbus::ObjectRegistryAsyncIPCDBusProxy* m_objectRegistryAsyncProxy = nullptr;
    DBusManagerInterface &m_dbusManager;
    bool m_initialized = false;
    std::unique_ptr<MasterImpl> m_master;
    Registry<QString> m_objects;
    int m_registryVersion = INVALID_REGISTRY_VERSION;

};

}
}
