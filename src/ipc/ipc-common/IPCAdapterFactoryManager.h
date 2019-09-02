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

#include <QPointer>
#include <QMap>
#include "NewIPCServiceAdapterBase.h"

#if defined(FaceliftIPCCommonLib_LIBRARY)
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_IMPORT
#endif

namespace facelift {

class NewIPCServiceAdapterBase;

class FaceliftIPCCommonLib_EXPORT IPCAdapterFactoryManager
{
public:
    typedef NewIPCServiceAdapterBase * (*IPCAdapterFactory)(InterfaceBase *);

    static IPCAdapterFactoryManager &instance();

    template<typename AdapterType>
    static NewIPCServiceAdapterBase *createInstance(InterfaceBase *i)
    {
        auto adapter = new AdapterType(i);
        adapter->setService(i);
        qCDebug(LogIpc) << "Created adapter for interface " << i->interfaceID();
        return adapter;
    }

    template<typename AdapterType>
    static void registerType()
    {
        instance().registerType(AdapterType::TheServiceType::FULLY_QUALIFIED_INTERFACE_NAME,  &IPCAdapterFactoryManager::createInstance<AdapterType>);
    }

    void registerType(const QString &typeID, IPCAdapterFactory f);

    IPCAdapterFactory getFactory(const QString &typeID) const;

private:
    QMap<QString, IPCAdapterFactory> m_factories;
};


class FaceliftIPCCommonLib_EXPORT IPCAttachedPropertyFactoryBase : public QObject
{
    Q_OBJECT

public:
    IPCAttachedPropertyFactoryBase(QObject *parent);

    static IPCServiceAdapterBase *qmlAttachedProperties(QObject *object);

    static InterfaceBase *getProvider(QObject *object);

};


class FaceliftIPCCommonLib_EXPORT IPCAttachedPropertyFactory : public IPCAttachedPropertyFactoryBase
{
public:
    static NewIPCServiceAdapterBase *qmlAttachedProperties(QObject *object);

};


}

QML_DECLARE_TYPEINFO(facelift::IPCAttachedPropertyFactory, QML_HAS_ATTACHED_PROPERTIES)

