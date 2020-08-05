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

#include "FaceliftCommon.h"
#include "FaceliftModel.h"

#include "StringConversionHandler.h"

#include "PropertyBase.h"
#include "TProperty.h"
#include "Property.h"
#include "ServiceProperty.h"
#include "PropertyConnector.h"//TODO - move these includes outside the file.

namespace facelift {

template<typename ElementType>
class ModelProperty : public Model<ElementType>, public PropertyBase
{
public:
    typedef std::function<ElementType(int)> ElementGetter;

    ModelProperty()
    {
    }

    template<typename Class>
    ModelProperty &bind(const ModelPropertyInterface<Class, ElementType> &property)
    {
        facelift::Model<ElementType>* model = property.property();
        this->bindOtherModel(model);

        this->beginResetModel();
        this->reset(property.property()->size(), [model](int index) {
            return model->elementAt(index);
        });
        this->endResetModel();
        return *this;
    }

    ElementType elementAt(int index) const override
    {
        Q_ASSERT(m_elementGetter);
        Q_ASSERT(!this->m_resettingModel);
        Q_ASSERT(index < this->size());
        return m_elementGetter(index);
    }

    void reset(int size, ElementGetter getter)
    {
        this->setSize(size);
        m_elementGetter = getter;
    }

    bool isDirty() const override
    {
        return m_modified;
    }

    void clean() override
    {
        m_modified = false;
    }

    QString toString() const override
    {
        return QString("Model ") + name();
    }

private:
    ElementGetter m_elementGetter;
    bool m_modified = false;
};


}
