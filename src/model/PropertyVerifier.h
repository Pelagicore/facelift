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

#include <QString>
#include <QObject>
#include <functional>

#include "FaceliftModel.h"

namespace facelift {

class FaceliftModelLib_EXPORT PropertyVerifierBase
{
public:
};

/**
 * This class can be used to verify the behavior of a property
 */
template<typename OwnerType, typename PropertyType>
class PropertyVerifier : public PropertyVerifierBase
{
    typedef const PropertyType& (OwnerType::*GetterMethod)() const;
    typedef void (OwnerType::*ChangeSignal)();

public:

    PropertyVerifier(OwnerType *i, GetterMethod getter, ChangeSignal changeSignal)
    {
        m_owner = i;

        m_previousValue = (m_owner->*getter)();
        QObject::connect(i, changeSignal, m_owner, [this, getter]() {
            auto newValue = (m_owner->*getter)();
            if (newValue == m_previousValue) {
                qCCritical(LogModel) << "Change signal triggered but the value has not changed :" << newValue << m_owner;
            }
            m_previousValue = newValue;
        });
    }

    OwnerType * m_owner = nullptr;
    PropertyType m_previousValue;
};

template<typename OwnerType, typename PropertyType>
static void createPropertyVerifier(const PropertyInterface<OwnerType, PropertyType>& property)
{
    new PropertyVerifier<OwnerType, PropertyType>(property.object, property.getter, property.signal);
}

template<typename OwnerType, typename PropertyType>
static void createPropertyVerifier(const ServicePropertyInterface<OwnerType, PropertyType>& property)
{
    Q_UNUSED(property);    // TODO: implement
}

template<typename OwnerType, typename PropertyType>
static void createPropertyVerifier(const ModelPropertyInterface<OwnerType, PropertyType>& property)
{
    Q_UNUSED(property);    // TODO: implement
}

}
