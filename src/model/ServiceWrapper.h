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

#include "FaceliftModel.h"

namespace facelift {


class FaceliftModelLib_EXPORT ServiceWrapperBase
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

/**
 * This class is used to write an interface implementation which delegates all calls to another implementation
 */
template<typename WrappedType>
class ServiceWrapper : public WrappedType, public ServiceWrapperBase
{

public:
    bool ready() const override
    {
        return m_wrapped ? m_wrapped->ready() : false;
    }

protected:
    ServiceWrapper(QObject *parent = nullptr) : WrappedType(parent)
    {
    }

    virtual void bind(WrappedType *wrapped, WrappedType *previouslyWrapped) = 0;

    WrappedType *wrapped() const
    {
        Q_ASSERT(m_wrapped);
        return m_wrapped;
    }

    void setWrapped(WrappedType *wrapped)
    {
        if (wrapped != m_wrapped) {
            bool wasReady = ready();
            WrappedType *previouslyWrapped = m_wrapped;
            __clearConnections__();
            m_wrapped = wrapped;
            addConnection(QObject::connect(this->wrapped(), &WrappedType::readyChanged, this, &ServiceWrapper::readyChanged));
            facelift::ServiceWrapperBase::setWrapped(*this, m_wrapped);
            bind(wrapped, previouslyWrapped);
            if (wasReady != ready()) {
                emit this->readyChanged();
            }
        }
    }

private:

    /**
     * Clear connections established to the currently wrapped object.
     * The name of this method is chosen in order to prevent from overriding any of the base class methods
     */
    void __clearConnections__() {
        if (m_wrapped)
            clearConnections();
    }

    QPointer<WrappedType> m_wrapped;

};

}
