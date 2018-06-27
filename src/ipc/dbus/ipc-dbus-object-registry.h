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

#include "ipc-dbus.h"
#include "facelift/ipc/ObjectRegistryIPC.h"

#include <QDBusContext>

namespace facelift {
namespace dbus {

class DBusObjectRegistry : public facelift::ipc::ObjectRegistryPropertyAdapter
{

    Q_OBJECT

public:
    static constexpr const char *SERVICE_NAME = "facelift.registry";

    DBusObjectRegistry(DBusManager &dbusManager) :
        m_dbusManager(dbusManager)
    {
    }

    void init()
    {
        if (!m_initialized) {
            m_initialized = true;
            if (m_dbusManager.registerServiceName(SERVICE_NAME)) {
                m_objectRegistryAdapter = new facelift::ipc::ObjectRegistryIPCAdapter();
                m_objectRegistryAdapter->setService(this);
                m_objectRegistryAdapter->init();
            } else {
                m_objectRegistryProxy = new facelift::ipc::ObjectRegistryIPCProxy();
                m_objectRegistryProxy->ipc()->setServiceName(SERVICE_NAME);
                m_objectRegistryProxy->connectToServer();
                QObject::connect(m_objectRegistryProxy, &facelift::ipc::ObjectRegistry::objectsChanged, this, &DBusObjectRegistry::objectsChanged);
            }
        }
    }

    bool registerObject(QString objectPath, QString serviceName) override
    {
        init();

        if (isMaster()) {
            auto objects = m_objects.value();
            if (!objects.contains(objectPath)) {
                qDebug() << "Object registered at path" << objectPath << "service name:" << serviceName;
                objects[objectPath] = serviceName;
                m_objects = objects;
                return true;
            } else {
                qWarning() << "Object path is already registered" << objectPath;
                return false;
            }

        } else {
            return m_objectRegistryProxy->registerObject(objectPath, serviceName);
        }
    }

    const QMap<QString, QString> &objects() const override
    {
        if (isMaster()) {
            return facelift::ipc::ObjectRegistryPropertyAdapter::objects();
        } else {
            return m_objectRegistryProxy->objects();
        }
    }

    bool unregisterObject(QString objectPath, QString serviceName) override
    {
        init();
        if (isMaster()) {
            auto objects = m_objects.value();
            Q_ASSERT(objects[objectPath] == serviceName);
            auto r = (objects.remove(objectPath) != 0);
            m_objects = objects;
            return r;
        } else {
            return m_objectRegistryProxy->unregisterObject(objectPath, serviceName);
        }
    }

    bool isMaster() const
    {
        return (m_objectRegistryProxy == nullptr);
    }

private:
    facelift::ipc::ObjectRegistryIPCProxy *m_objectRegistryProxy = nullptr;
    facelift::ipc::ObjectRegistryIPCAdapter *m_objectRegistryAdapter = nullptr;
    DBusManager &m_dbusManager;
    bool m_initialized = false;
};

}
}
