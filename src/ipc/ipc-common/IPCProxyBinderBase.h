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
#include "ipc-common.h"

#if defined(FaceliftIPCCommonLib_LIBRARY)
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_IMPORT
#endif

namespace facelift {

class FaceliftIPCCommonLib_EXPORT IPCProxyBinderBase : public QObject
{
    Q_OBJECT

public:
    Q_PROPERTY(QString objectPath READ objectPath WRITE setObjectPath)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)
    Q_PROPERTY(QString serviceName READ serviceName WRITE setServiceName)
    Q_PROPERTY(QString address READ address WRITE setAddress)

    IPCProxyBinderBase(InterfaceBase &owner, QObject *parent);

    inline const QString &serviceName() const
    {
        return m_serviceName;
    }

    void setServiceName(const QString &name);

    bool enabled() const
    {
        return m_enabled;
    }

    void setEnabled(bool enabled);

    const QString &objectPath() const
    {
        return m_objectPath;
    }

    virtual void setObjectPath(const QString &objectPath);

    inline const QString& address()
    {
        return m_address;
    }
    virtual void setAddress(const QString& address);

    void onComponentCompleted();

    void checkInit();

    bool isReadyToConnect() const;

    /**
     * Establish the connection with the server
     */
    void connectToServer();

    virtual void bindToIPC()
    {

    }

    Q_SIGNAL void complete();

    Q_SIGNAL void serviceAvailableChanged();

    virtual bool isServiceAvailable() const;

    InterfaceBase &owner()
    {
        return m_owner;
    }

    template<typename SubInterfaceProxyType>
    SubInterfaceProxyType *getOrCreateSubProxy(const QString &objectPath)
    {
        if (objectPath.isEmpty()) {
            return nullptr;
        }

        if (m_subProxies.contains(objectPath)) {
            return qobject_cast<SubInterfaceProxyType *>(&(m_subProxies[objectPath]->owner()));
        }

        auto proxy = new SubInterfaceProxyType();
        proxy->ipc()->setObjectPath(objectPath);
        proxy->connectToServer();

        m_subProxies.insert(objectPath, proxy->ipc());

        return proxy;
    }

    void setSynchronous(bool isSynchronous)
    {
        m_isSynchronous = isSynchronous;
    }

    bool isSynchronous() const
    {
        return m_isSynchronous;
    }

protected:
    bool m_explicitServiceName = false;
    QString m_serviceName;

private:
    QMap<QString, IPCProxyBinderBase *> m_subProxies;
    QString m_objectPath;
    InterfaceBase &m_owner;
    bool m_alreadyInitialized = false;
    bool m_enabled = true;
    QString m_address;
    bool m_componentCompleted = false;
    bool m_isSynchronous = true;
};

}
