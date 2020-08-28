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

#include "LocalIPCMessage.h"
#include "IPCProxyBase.h"
#include "LocalIPCRequestHandler.h"

#if defined(FaceliftIPCLocalLib_LIBRARY)
#  define FaceliftIPCLocalLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCLocalLib_EXPORT Q_DECL_IMPORT
#endif

namespace facelift {

namespace local {

using namespace facelift;

class LocalIPCServiceAdapterBase;

class FaceliftIPCLocalLib_EXPORT LocalIPCProxyBinder : public IPCProxyBinderBase
{
    Q_OBJECT

public:
    LocalIPCProxyBinder(InterfaceBase &owner, QObject *parent = nullptr);

    const QString &serviceName() const;

    void setObjectPath(const QString &objectPath) override;

    void setServiceName(const QString &name);

    const QString &interfaceName() const;

    void setInterfaceName(const QString &name);

    void onPropertiesChanged(LocalIPCMessage &message);

    void bindToIPC() override;

    void setServiceAvailable(bool isRegistered);

    void checkServiceAvailability();

    void notifyServiceAvailable();

    bool isServiceAvailable() const override;

    void requestPropertyValues();

    void onServerNotAvailableError(const QString &methodName) const;

    template<typename PropertyType>
    void sendSetterCall(const QString &property, const PropertyType &value);

    template<typename ... Args>
    LocalIPCMessage sendMethodCall(const char *methodName, const Args & ... args) const;

    template<typename ReturnType, typename ... Args>
    void sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<ReturnType> answer, const Args & ... args);

    template<typename ... Args>
    void sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<void> answer, const Args & ... args);

    template<typename ReturnType, typename ... Args>
    void sendMethodCallWithReturn(const char *methodName, ReturnType &returnValue, const Args & ... args) const;

    LocalIPCMessage call(LocalIPCMessage &message) const;

    void asyncCall(LocalIPCMessage &requestMessage, QObject *context, std::function<void(LocalIPCMessage &message)> callback);

    void setHandler(LocalIPCRequestHandler *handler);

private:
    QString m_serviceName;
    QString m_interfaceName;
    LocalIPCRequestHandler *m_serviceObject = nullptr;
    QPointer<LocalIPCServiceAdapterBase> m_serviceAdapter;
    QMetaObject::Connection m_signalConnection;
    bool m_explicitServiceName = false;
    bool m_serviceAvailable = false;
};

 
}

}
