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

#include "TProperty.h"
#include "ServicePropertyInterface.h"

namespace facelift {

/**
 * Specialization used to store a reference to an interface.
 */
template<typename Type>
class ServiceProperty : public TProperty<Type *>
{

public:
    template<typename Class, typename PropertyType>
    ServiceProperty &bind(const ServicePropertyInterface<Class, PropertyType> &property)
    {
        this->bind([property] () {
                return property.value();
            }).addTrigger(property.object(), property.signal());
        return *this;
    }

    Type *operator=(Type *right)
    {
        ServiceProperty::setValue(right);
        return this->m_value;
    }

    void setValue(Type *newValue)
    {
        if (newValue != m_pointer) {
            m_valueChanged = true;
            // Store as a QPointer in order to be able to detect object destructions and generate a change signal even if the same
            // pointer is assigned later on
            m_pointer = newValue;
        }

        TProperty<Type *>::setValue(newValue);
    }

    bool isDirty() const override
    {
        return ((this->m_previousValue != this->value()) || m_valueChanged);
    }

    void clean() override
    {
        TProperty<Type *>::clean();
        m_valueChanged = false;
    }

    using TProperty<Type *>::bind;

private:
    bool m_valueChanged = false;

    QPointer<Type> m_pointer;
};

}
