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

#include "FaceliftModel.h"

namespace facelift {


class ServiceWrapperBase
{

protected:
    void addConnection(QMetaObject::Connection connection);

    void clearConnections();

    void setWrapped(InterfaceBase &wrapper, InterfaceBase *wrapped);

    /// Normal property
    template<typename PropertyType, typename InterfaceType>
    static bool changeSignalRequired(const PropertyType& (InterfaceType::*getter)() const, const InterfaceType* newService, const InterfaceType* previousService, const PropertyType& previousValue)
    {
        Q_ASSERT(newService != previousService);
        Q_ASSERT(newService != nullptr);
        const auto & referenceValue = ((previousService == nullptr) ? previousValue : (previousService->*getter)());
        return ((newService->*getter)() != referenceValue);
    }

    /// Model property
    template<typename PropertyType, typename InterfaceType>
    static bool changeSignalRequired(PropertyType& (InterfaceType::*getter)(), InterfaceType* newService, InterfaceType* previousService, const PropertyType* previousValue)
    {
        M_UNUSED(getter, newService, previousService, previousValue);
        Q_ASSERT(newService != previousService);
        Q_ASSERT(newService != nullptr);
        return true;   // two distinct service instances may not share the same model instance
    }

    /// Interface property
    template<typename PropertyType, typename InterfaceType>
    static bool changeSignalRequired(PropertyType* (InterfaceType::*getter)(), InterfaceType* newService, InterfaceType* previousService, const PropertyType* previousValue)
    {
        Q_ASSERT(newService != previousService);
        Q_ASSERT(newService != nullptr);
        const auto referenceValue = ((previousService == nullptr) ? previousValue : (previousService->*getter)());
        return ((newService->*getter)() != referenceValue);
    }

private:
    QVector<QMetaObject::Connection> m_connections;  /// The list of connections which this property is bound to

};


}
