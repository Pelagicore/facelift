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

#include <QString>
#include <QObject>

#include <functional>

#include "PropertyBase.h"
#include "PropertyInterface.h"
#include "StringConversionHandler.h"

namespace facelift {

/**
 * This template helper class wraps a value and a "value changed" signal which should be triggered whenever the value is changed.
 * When changing the value of the property, the user of this class does not need to explicitly trigger the signal since this is
 * done automatically for him.
 */
template<typename Type>
class TProperty : public PropertyBase
{

    typedef std::function<Type()> GetterFunction;

public:
    TProperty(Type initialValue)
    {
        m_value = m_previousValue = initialValue;
    }

    TProperty()
    {
    }

    QString toString() const override
    {
        return facelift::toString(value());
    }

    template<typename Class, typename PropertyType>
    TProperty &bind(const PropertyInterface<Class, PropertyType> &property)
    {
        this->bind([property] () {
                return property.value();
            }).addTrigger(property.object(), property.signal());
        return *this;
    }

    /**
     * Add the given property to the properties which "this" property is bound to, which means that the value of "this" property will
     * be reevaluated whenever the signal is triggered
     */
    template<typename Class, typename PropertyType>
    TProperty &addTrigger(const PropertyInterface<Class, PropertyType> &property)
    {
        this->addTrigger(property.object(), property.signal());
        return *this;
    }

    /**
     * Add the given signal to the signals which this property is bound to, which means that the value of the property will
     * be reevaluated whenever the signal is triggered
     */
    template<typename SourceObjectType, typename SourceSignalType, typename ... Args>
    TProperty &addTrigger(const SourceObjectType *source, void (SourceSignalType::*changeSignal)(Args ...))
    {
        m_connections.push_back(QObject::connect(source, changeSignal, owner(), [this]() {
                reevaluate();
            }));
        return *this;
    }

    const Type &value() const
    {
        return m_value;
    }

    operator const Type &() const
    {
        return value();
    }

    TProperty &bind(const GetterFunction &f)
    {
        breakBinding();

        m_getterFunction = f;
        reevaluate();

        return *this;
    }

    void setValue(const Type &right)
    {
        breakBinding();

        m_value = right;
        qCDebug(LogModel) << "Written value to property " << name() << ":" << toString();
        triggerValueChangedSignal();
    }

    Type &operator=(const Type &right)
    {
        setValue(right);
        return m_value;
    }

    bool isDirty() const override
    {
        return !(m_previousValue == m_value);
    }

    Type &operator-=(const Type &right)
    {
        return operator=(value() - right);
    }

    Type &operator+=(const Type &right)
    {
        return operator=(value() + right);
    }

    Type &operator*=(const Type &right)
    {
        return operator=(value() * right);
    }

    Type &operator/=(const Type &right)
    {
        return operator=(value() / right);
    }

    Type operator++(int)
    {
        m_value++;
        triggerValueChangedSignal();
        return m_value;
    }

    Type operator--(int)
    {
        m_value--;
        triggerValueChangedSignal();
        return m_value;
    }

protected:

    void clean() override
    {
        m_previousValue = m_value;
        //        qCDebug(LogModel) << "Cleaning " << name() << " value: " << m_previousValue;
    }

    void breakBinding()
    {
        if (m_getterFunction) {
            m_getterFunction = nullptr;
            doBreakBinding();
        }
    }

    Type m_value = {};  /// The current value

protected:
    void reevaluate()
    {
        Q_ASSERT(m_getterFunction);
        m_value = m_getterFunction();
        triggerValueChangedSignal();
    }

    Type m_previousValue = m_value;  /// The value when the last "value changed" signal was triggered

    GetterFunction m_getterFunction;  /// The bound getter function, if any

};

}
