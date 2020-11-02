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
#include <QQmlEngine>

#include "QMLAdapterBase.h"
#include "QMLAdapterByReference.h"
#include "TQMLAdapter.h"
#include "ModelListModel.h"

namespace facelift {

class IPCAttachedPropertyFactory;
class IPCServiceAdapterBase;


template<typename Type>
QObject *singletonGetter(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(scriptEngine);
    Q_UNUSED(engine);
    auto obj = new Type(engine);
    obj->componentComplete();
    qCDebug(LogModel) << "Singleton created" << obj;
    return obj;
}


template<typename ProviderType, ProviderType& (*getter)()>
QObject *singletonGetterByFunction(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(scriptEngine);
    Q_UNUSED(engine);
    auto obj = new QMLAdapterByReference<ProviderType>(getter());
    obj->connectProvider();
    obj->componentComplete();
    qCDebug(LogModel) << "Singleton created" << obj;
    return obj;
}


/*!
 * Register the given C++ interface implementation as an uncreatable QML type.
 * By default, the type is registered under the same name as defined in the QFace definition.
 */
template<typename ProviderType>
int registerUncreatableQmlComponent(const char *uri, const char *name = ProviderType::INTERFACE_NAME,
        int majorVersion = ProviderType::VERSION_MAJOR, int minorVersion = ProviderType::VERSION_MINOR,
        typename std::enable_if<std::is_base_of<facelift::InterfaceBase, ProviderType>::value>::type * = nullptr)
{
    return ::qmlRegisterUncreatableType<typename ProviderType::QMLAdapterType>(uri, majorVersion, minorVersion,
                                                                                name, "");
}


/*!
 * Register the given C++ interface implementation as a creatable QML component.
 * By default, the component is registered under the same name as defined in the QFace definition.
 */
template<typename ProviderType>
int registerQmlComponent(const char *uri, const char *name = ProviderType::INTERFACE_NAME, int majorVersion =
        ProviderType::VERSION_MAJOR,
        int minorVersion = ProviderType::VERSION_MINOR,
        typename std::enable_if<std::is_base_of<facelift::InterfaceBase, ProviderType>::value>::type * = nullptr)
{
    ProviderType::registerTypes(uri);
    return ::qmlRegisterType<TQMLAdapter<ProviderType> >(uri, majorVersion, minorVersion, name);
}


/*!
 * Register the given implementation type as a singleton QML component.
 * By default, the component is registered under the same name as defined in the QFace definition.
 */
template<typename ProviderType>
int registerSingletonQmlComponent(const char *uri,
        const char *name = ProviderType::INTERFACE_NAME,
        int majorVersion = ProviderType::VERSION_MAJOR,
        int minorVersion = ProviderType::VERSION_MINOR,
        typename std::enable_if<std::is_base_of<facelift::InterfaceBase, ProviderType>::value>::type * = nullptr)
{
    ProviderType::registerTypes(uri);
    typedef TQMLAdapter<ProviderType> QMLType;
    return ::qmlRegisterSingletonType<QMLType>(uri, majorVersion, minorVersion, name, &singletonGetter<QMLType>);
}


/*!
 * Register the given implementation type as a singleton QML component, with the given getter function.
 * By default, the component is registered under the same name as defined in the QFace definition.
 */
template<typename ProviderType, ProviderType& (*singletonGetterFunction)()>
int registerSingletonQmlComponent(const char *uri,
        const char *name = ProviderType::INTERFACE_NAME,
        int majorVersion = ProviderType::VERSION_MAJOR,
        int minorVersion = ProviderType::VERSION_MINOR,
        typename std::enable_if<std::is_base_of<facelift::InterfaceBase, ProviderType>::value>::type * = nullptr)
{
    ProviderType::registerTypes(uri);
    typedef QMLAdapterByReference<ProviderType> QMLType;
    return ::qmlRegisterSingletonType<QMLType>(uri, majorVersion, minorVersion, name,
                                               &singletonGetterByFunction<ProviderType, singletonGetterFunction>);
}


template<typename ProviderType>
typename ProviderType::QMLAdapterType *getQMLAdapter(ProviderType *provider)
{
    if (provider == nullptr) {
        return nullptr;
    } else {
        if (provider->m_qmlAdapter == nullptr) {
            // No QML adapter instantiated yet => create one
            provider->m_qmlAdapter = new typename ProviderType::QMLAdapterType(provider);
            provider->m_qmlAdapter->connectProvider(*provider);
        }
        return provider->m_qmlAdapter;
    }
}


}
