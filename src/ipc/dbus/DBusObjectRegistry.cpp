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

#include "DBusObjectRegistry.h"
#include "DBusIPCCommon.h"
#include <limits>

namespace facelift {
namespace dbus {

using facelift::ipc::dbus::ObjectRegistryIPCDBusProxy;
using facelift::ipc::dbus::ObjectRegistryAsyncIPCDBusProxy;
using facelift::ipc::dbus::ObjectRegistryAsync;

const QString DBusObjectRegistry::VERSION_KEY = "@version";

DBusObjectRegistry::DBusObjectRegistry(DBusManagerInterface &dbusManager) :
    m_dbusManager(dbusManager), m_objects(this)
{
    const constexpr char* ENV_VAR_NAME = "FACELIFT_DBUS_SERVICE_NAME";
    QByteArray serviceName = qgetenv(ENV_VAR_NAME);
    m_serviceName = serviceName.isEmpty() ? DBusIPCCommon::DEFAULT_SERVICE_NAME : QString(serviceName);
}

void DBusObjectRegistry::init()
{
    if (!m_initialized) {
        m_initialized = true;
        if (!QDBusConnection::sessionBus().isConnected() || QDBusConnection::sessionBus().registerService(m_serviceName)) {
            m_master = std::make_unique<MasterImpl>();
            m_master->init();
            QObject::connect(m_master.get(), &MasterImpl::objectAdded, this, &DBusObjectRegistry::onObjectAdded);
            QObject::connect(m_master.get(), &MasterImpl::objectRemoved, this, &DBusObjectRegistry::onObjectRemoved);
            updateObjects(m_master->getObjects());
        } else {
            m_objectRegistryAsyncProxy = new ObjectRegistryAsyncIPCDBusProxy(this);
            m_objectRegistryAsyncProxy->ipc()->setServiceName(m_serviceName);
            m_objectRegistryAsyncProxy->connectToServer();
            m_objectRegistryAsyncProxy->getObjects(facelift::AsyncAnswer<QMap<QString, QString>>(
                this, [this](QMap<QString, QString> objectMap) { updateObjects(objectMap); }));
            QObject::connect(m_objectRegistryAsyncProxy, &ObjectRegistryAsync::objectAdded, this,
                             &DBusObjectRegistry::onObjectAdded);
            QObject::connect(m_objectRegistryAsyncProxy, &ObjectRegistryAsync::objectRemoved, this,
                             &DBusObjectRegistry::onObjectRemoved);
        }
    }
}

void DBusObjectRegistry::registerObject(const QString &objectPath, const QString& serviceName, facelift::AsyncAnswer<bool> answer)
{
    init();
    if (isMaster()) {
        auto isSuccessful = m_master->registerObject(objectPath, serviceName);
        answer(isSuccessful);
    } else {
        m_objectRegistryAsyncProxy->registerObject(objectPath, serviceName, answer);
    }
}

void DBusObjectRegistry::unregisterObject(const QString &objectPath, const QString& serviceName)
{
    init();
    if (isMaster()) {
        m_master->unregisterObject(objectPath, serviceName);
    } else {
        m_objectRegistryAsyncProxy->unregisterObject(objectPath, serviceName);
    }
}

void DBusObjectRegistry::updateObjects(const QMap<QString, QString>& objectMap)
{
    if (!hasValidObjects()) {
        int version = objectMap[VERSION_KEY].toInt(); // 0 if key is not present
        Q_ASSERT(version != INVALID_REGISTRY_VERSION);
        m_registryVersion = version;
        auto cleanObjects = objectMap;
        cleanObjects.remove(VERSION_KEY);
        m_objects.setContent(cleanObjects);
    }
}

int DBusObjectRegistry::nextVersion(const int currentVersion)
{
    int version = currentVersion + 1;
    if (version == std::numeric_limits<int>::max()) { // overflow protection
        version = INITIAL_REGISTRY_VERSION;
    }
    return version;
}

void DBusObjectRegistry::syncObjects()
{
    ObjectRegistryIPCDBusProxy objectRegistryProxy;
    objectRegistryProxy.ipc()->setServiceName(m_serviceName);
    objectRegistryProxy.connectToServer();
    Q_ASSERT(objectRegistryProxy.ready());
    updateObjects(objectRegistryProxy.getObjects());
}

const Registry<QString>& DBusObjectRegistry::objects(bool blocking)
{
    init();
    if (!isMaster() && blocking && !hasValidObjects()) {
        syncObjects();
    }
    return m_objects;
}

void DBusObjectRegistry::onObjectAdded(const QString &objectPath, const QString &serviceName, int registryVersion)
{
    if (hasValidObjects() && registryVersion == nextVersion(m_registryVersion)) {
        m_registryVersion = registryVersion;
        if (!m_objects.contains(objectPath)) {
            m_objects.insert(objectPath, serviceName);
        } else {
            qCCritical(LogIpc) << "Cannot add object with object path:" << objectPath << ", service name:" << serviceName
                               << "Object registry already contains this object path with service name:" << m_objects[objectPath];
        }
    }
}

void DBusObjectRegistry::onObjectRemoved(const QString &objectPath, int registryVersion)
{
    if (hasValidObjects() && registryVersion == nextVersion(m_registryVersion)) {
        m_registryVersion = registryVersion;
        if (!m_objects.remove(objectPath)) {
            qCCritical(LogIpc) << "Object does not exist. Object path:" << objectPath;
        }
    }
}

bool DBusObjectRegistry::isMaster() const
{
    return m_master != nullptr;
}

void DBusObjectRegistry::MasterImpl::init()
{
    Q_ASSERT(m_version == DBusObjectRegistry::INVALID_REGISTRY_VERSION);
    m_version = DBusObjectRegistry::INITIAL_REGISTRY_VERSION;
    m_objectRegistryAdapter.registerService(facelift::ipc::dbus::ObjectRegistry::SINGLETON_OBJECT_PATH, this);
}

void DBusObjectRegistry::MasterImpl::updateVersion()
{
    m_version = nextVersion(m_version);
}

bool DBusObjectRegistry::MasterImpl::registerObject(const QString &objectPath, const QString &serviceName)
{
    Q_ASSERT_X(m_version != DBusObjectRegistry::INVALID_REGISTRY_VERSION, "registerObject",
               "Attempt to register object before the registry initialization");
    if (objectPath.isEmpty() || serviceName.isEmpty()) {
        qCCritical(LogIpc) << "Object path or service name is empty. Object path:" << objectPath
                           << "service name:" << serviceName;
        return false;
    } else if (!m_objectMap.contains(objectPath)) {
        m_objectMap.insert(objectPath, serviceName);
        qCDebug(LogIpc) << "Object registered at path" << objectPath
                        << "service name:" << serviceName;

        updateVersion();
        emit objectAdded(objectPath, serviceName, m_version);
        return true;
    } else {
        qCCritical(LogIpc) << "Object path is already registered" << objectPath;
        return false;
    }
}

bool DBusObjectRegistry::MasterImpl::unregisterObject(const QString &objectPath, const QString &serviceName)
{
    Q_ASSERT_X(m_version != DBusObjectRegistry::INVALID_REGISTRY_VERSION, "registerObject",
               "Attempt to unregister object before the registry initialization");
    if (m_objectMap.contains(objectPath) && m_objectMap[objectPath] == serviceName) {
        m_objectMap.remove(objectPath);
        updateVersion();
        emit objectRemoved(objectPath, m_version);
        return true;
    } else {
        qCCritical(LogIpc, "Could not unregister service at object path '%s' serviceName:'%s'", qPrintable(objectPath),
                   qPrintable(serviceName));
        return false;
    }
}

QMap<QString, QString> DBusObjectRegistry::MasterImpl::getObjects()
{
    QMap<QString, QString> result(m_objectMap);
    result[VERSION_KEY] = QString::number(m_version);
    return result;
}

} // end namespace dbus
} // end namespace facelift
