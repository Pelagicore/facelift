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
#include "NotAvailableImplBase.h"

namespace facelift {

template<typename InterfaceType>
class NotAvailableImpl : public InterfaceType, protected NotAvailableImplBase
{
public:
    template<typename ElementType>
    struct NotAvailableModel
    {
        static Model<ElementType> &value()
        {
            static TheModel instance;
            return instance;
        }

        class TheModel : public Model<ElementType>
        {
        public:
            ElementType elementAt(int index) const override
            {
                Q_UNUSED(index);
                Q_ASSERT(false);
                return ElementType {};
            }
        };

    };

    template<typename Type>
    struct NotAvailableValue
    {
        static const Type &value()
        {
            static Type instance = {};
            return instance;
        }
    };

    template<typename Type>
    struct NotAvailableList
    {
        static const QList<Type> &value()
        {
            static QList<Type> instance;
            return instance;
        }
    };

    template<typename Type>
    void logSetterCall(const char *propertyName, const Type &value) const
    {
        Q_UNUSED(value);
        NotAvailableImplBase::logSetterCall(*this, propertyName);
    }

    void logGetterCall(const char *propertyName) const
    {
        NotAvailableImplBase::logGetterCall(*this, propertyName);
    }

    template<typename ... Args>
    void logMethodCall(const char *methodName, const Args & ... args) const
    {
        M_UNUSED(args ...);
        NotAvailableImplBase::logMethodCall(*this, methodName);
    }

    bool ready() const override
    {
        return false;
    }

};

}
