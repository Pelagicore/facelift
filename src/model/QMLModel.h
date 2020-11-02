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

#include "ModelQMLImplementation.h"
#include "QMLModelImplementationFrontend.h"
#include "QObjectWrapperPointer.h"

namespace facelift {

/*!
 * Register the given interface QML implementation as a creatable QML component.
 * By default, the component is registered under the same name as defined in the Qface definition.
 * \param qmlFilePath Path of the file containing the QML implementation of the interface
 */
template<typename ImplementationBaseQMLType>
int registerQmlComponent(const char *uri, const char *qmlFilePath,
        const char *componentName = ImplementationBaseQMLType::Provider::INTERFACE_NAME,
        int majorVersion = ImplementationBaseQMLType::Provider::VERSION_MAJOR,
        int minorVersion = ImplementationBaseQMLType::Provider::VERSION_MINOR,
        typename std::enable_if<std::is_base_of<facelift::ModelQMLImplementationBase, ImplementationBaseQMLType>::value>::type * = nullptr)
{
    //    qCDebug(LogModel) << "Registering QML implementation \"" << qmlFilePath << "\" for component \"" << componentName << "\"";
    ImplementationBaseQMLType::Provider::registerTypes(uri);
    ImplementationBaseQMLType::setModelImplementationFilePath(qmlFilePath);
    return ::qmlRegisterType<TQMLAdapter<typename ImplementationBaseQMLType::Provider> >(uri, majorVersion, minorVersion, componentName);
}

/*!
 * Register the given interface QML implementation as QML singleton.
 * By default, the component is registered under the same name as defined in the Qface definition.
 * \param qmlFilePath Path of the file containing the QML implementation of the interface
 */
template<typename ImplementationBaseQMLType>
int registerSingletonQmlComponent(const char *uri, const char *qmlFilePath,
        const char *name = ImplementationBaseQMLType::Provider::QMLAdapterType::INTERFACE_NAME,
        int majorVersion = ImplementationBaseQMLType::Provider::VERSION_MAJOR,
        int minorVersion = ImplementationBaseQMLType::Provider::VERSION_MINOR,
        typename std::enable_if<std::is_base_of<facelift::ModelQMLImplementationBase, ImplementationBaseQMLType>::value>::type * = nullptr)
{
    ImplementationBaseQMLType::Provider::registerTypes(uri);
    ImplementationBaseQMLType::setModelImplementationFilePath(qmlFilePath);
    typedef TQMLAdapter<typename ImplementationBaseQMLType::Provider> QMLType;
    return ::qmlRegisterSingletonType<QMLType>(uri, majorVersion, minorVersion, name, &singletonGetter<QMLType>);
}

}
