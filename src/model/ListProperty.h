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

#include <QList>

#include "FaceliftCommon.h"
#include "FaceliftModel.h"

#include "StringConversionHandler.h"

#include "TProperty.h"

namespace facelift {

template<typename ElementType>
class ListProperty : public TProperty<QList<ElementType> >
{

public:
    using TProperty<QList<ElementType> >::operator=;

    void removeAt(int i)
    {
        this->modifiableValue().removeAt(i);
        this->triggerValueChangedSignal();
    }

    void addElement(ElementType element)
    {
        this->modifiableValue().append(element);
        this->triggerValueChangedSignal();
    }

    int size() const
    {
        return this->value().size();
    }

private:
    QList<ElementType> &modifiableValue()
    {
        this->breakBinding();
        return this->m_value;
    }

};


}
