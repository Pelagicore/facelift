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


#include <QObject>
#include <QList>
#include <QVariant>

#include "TProperty.h"
#include "QMLImplListPropertyBase.h"

namespace facelift {

template<typename ElementType>
class TQMLImplListProperty : public QMLImplListPropertyBase
{

public:
    TProperty<QList<ElementType> > &property() const
    {
        Q_ASSERT(m_property != nullptr);
        return *m_property;
    }

    QList<QVariant> elementsAsVariant() const override
    {
        return facelift::toQMLCompatibleType(elements());
    }

    void onReferencedObjectChanged()
    {
        // TODO: check that QObject references in QVariant
        // One of the referenced objects has emitted a change signal so we refresh our list
        refreshList(m_assignedVariantList);
    }

    void clearConnections()
    {
        for (const auto &connection : m_changeSignalConnections) {
            auto successfull = QObject::disconnect(connection);
            Q_ASSERT(successfull);
        }
        m_changeSignalConnections.clear();
    }

    void refreshList(const QList<QVariant> &variantList)
    {
        auto list = m_property->value();
        list.clear();

        clearConnections();
        for (const auto &var : variantList) {
            list.append(fromVariant<ElementType>(var));

            // Add connections so that we can react when the property of an object has changed
            TypeHandler<ElementType>::connectChangeSignals(var, this,
                    &TQMLImplListProperty::onReferencedObjectChanged,
                    m_changeSignalConnections);
        }

        m_property->setValue(list);
        elementsChanged();
    }

    void setElementsAsVariant(const QList<QVariant> &variantList) override
    {
        m_assignedVariantList = variantList;
        refreshList(m_assignedVariantList);
    }

    void setProperty(TProperty<QList<ElementType> > &property)
    {
        if (m_property == nullptr) {
            m_property = &property;
        }
    }

    int size() const override
    {
        return property().value().size();
    }

    const QList<ElementType> &elements() const
    {
        return property().value();
    }

private:
    TProperty<QList<ElementType> > *m_property = nullptr;
    QList<QVariant> m_assignedVariantList;
    QList<QMetaObject::Connection> m_changeSignalConnections;
};


}
