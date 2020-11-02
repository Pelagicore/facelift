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

#include <QMap>
#include <QString>
#include <QList>

#include "ModelProperty.h"
#include "FaceliftConversion.h"
#include "FaceliftQMLUtils.h"

#include "Property.h"

#include "QMLImplMapPropertyBase.h"

namespace facelift {

template<typename ElementType>
class TQMLImplMapProperty : public QMLImplMapPropertyBase
{

public:
    Property<QMap<QString, ElementType> > &property() const
    {
        Q_ASSERT(m_property != nullptr);
        return *m_property;
    }

    QVariantMap elementsAsVariant() const override
    {
        return toQMLCompatibleType(elements());
    }

    void onReferencedObjectChanged()
    {
        // TODO: check that QObject references in QVariant
        // One of the referenced objects has emitted a change signal so we refresh our map
        refreshMap(m_assignedVariantMap);
    }

    void clearConnections()
    {
        for (const auto &connection : m_changeSignalConnections) {
            auto successfull = QObject::disconnect(connection);
            Q_ASSERT(successfull);
        }
        m_changeSignalConnections.clear();
    }

    void refreshMap(const QVariantMap &variantMap)
    {
        auto map = m_property->value();
        map.clear();

        clearConnections();
        for (auto i = variantMap.constBegin(); i != variantMap.constEnd(); ++i) {
            map.insert(i.key(), fromVariant<ElementType>(i.value()));
            // Add connections so that we can react when the property of an object has changed
            TypeHandler<ElementType>::connectChangeSignals(i.value(), this,
                    &TQMLImplMapProperty::onReferencedObjectChanged,
                    m_changeSignalConnections);
        }

        m_property->setValue(map);
        elementsChanged();
    }

    void setElementsAsVariant(const QVariantMap &variantMap) override
    {
        m_assignedVariantMap = variantMap;
        refreshMap(m_assignedVariantMap);
    }

    void setProperty(Property<QMap<QString, ElementType> > &property)
    {
        if (m_property == nullptr) {
            m_property = &property;
        }
    }

    int size() const override
    {
        return property().value().size();
    }

    const QMap<QString, ElementType> &elements() const
    {
        return property().value();
    }

private:
    Property<QMap<QString, ElementType> > *m_property = nullptr;
    QVariantMap m_assignedVariantMap;
    QList<QMetaObject::Connection> m_changeSignalConnections;
};


}
