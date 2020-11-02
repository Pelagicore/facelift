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
#include "ServiceWrapperBase.h"

namespace facelift {


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
