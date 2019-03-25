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

#include "ipc-dbus-object-registry.h"
#include "FaceliftLogging.h"

namespace facelift {
namespace dbus {

void DBusObjectRegistry::init()
{
    if (!m_initialized) {
        m_initialized = true;
        if (m_dbusManager.registerServiceName(SERVICE_NAME)) {
            m_master = std::make_unique<MasterImpl>(*this);
            m_master->init();
            QObject::connect(m_master.get(), &MasterImpl::objectsChanged, this, &DBusObjectRegistry::objectsChanged);
        } else {
            m_objectRegistryAsyncProxy = new facelift::ipc::dbus::ObjectRegistryAsyncIPCDBusProxy();
            m_objectRegistryAsyncProxy->ipc()->setServiceName(SERVICE_NAME);
            m_objectRegistryAsyncProxy->connectToServer();
            QObject::connect(m_objectRegistryAsyncProxy, &facelift::ipc::dbus::ObjectRegistryAsync::objectsChanged, this,
                    &DBusObjectRegistry::objectsChanged);
        }
    }
}

void DBusObjectRegistry::registerObject(const QString &objectPath, facelift::AsyncAnswer<bool> answer)
{
    init();
    auto serviceName = DBusManager::instance().serviceName();
    if (m_master) {
        auto isSuccessful = m_master->registerObject(objectPath, serviceName);
        answer(isSuccessful);
    } else {
        return m_objectRegistryAsyncProxy->registerObject(objectPath, serviceName, answer);
    }
}

void DBusObjectRegistry::unregisterObject(const QString &objectPath)
{
    init();
    auto serviceName = DBusManager::instance().serviceName();
    if (m_master) {
        m_master->unregisterObject(objectPath, serviceName);
    } else {
        return m_objectRegistryAsyncProxy->unregisterObject(objectPath, serviceName);
    }
}

const QMap<QString, QString> &DBusObjectRegistry::objects(bool blocking)
{
    init();
    if (m_master) {
        return m_master->objects();
    } else {
        if (blocking) {
            if (m_objectRegistryProxy == nullptr) {
                m_objectRegistryProxy = new facelift::ipc::dbus::ObjectRegistryIPCDBusProxy();
                m_objectRegistryProxy->ipc()->setServiceName(SERVICE_NAME);
                m_objectRegistryProxy->connectToServer();
            }
            return m_objectRegistryProxy->objects();
        } else {
            return m_objectRegistryAsyncProxy->objects();
        }
    }
}

void DBusObjectRegistry::MasterImpl::init()
{
    m_objectRegistryAdapter.registerService(facelift::ipc::dbus::ObjectRegistry::SINGLETON_OBJECT_PATH, this);
}

bool DBusObjectRegistry::MasterImpl::registerObject(QString objectPath, QString serviceName)
{
    auto objects = m_objects.value();
    if (!objects.contains(objectPath)) {
        qDebug() << "Object registered at path" << objectPath << "service name:" << serviceName;
        objects[objectPath] = serviceName;
        m_objects = objects;
        return true;
    } else {
        qCCritical(LogIpc) << "Object path is already registered" << objectPath;
        return false;
    }
}

bool DBusObjectRegistry::MasterImpl::unregisterObject(QString objectPath, QString serviceName)
{
    auto objects = m_objects.value();
    Q_ASSERT(objects[objectPath] == serviceName);
    auto r = (objects.remove(objectPath) != 0);
    m_objects = objects;
    return r;
}


}
}
