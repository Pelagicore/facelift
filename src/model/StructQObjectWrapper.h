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

#include "FaceliftProperty.h"
#include "FaceliftConversion.h"
#include "FaceliftQMLUtils.h"

#if defined(FaceliftModelLib_LIBRARY)
#  define FaceliftModelLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftModelLib_EXPORT Q_DECL_IMPORT
#endif

namespace facelift {


class FaceliftModelLib_EXPORT StructQObjectWrapperBase : public QObject
{
    Q_OBJECT

public:
    StructQObjectWrapperBase(QObject *parent = nullptr) : QObject(parent)
    {
    }

    /**
     * This signal is triggered when one of the fields has changed
     */
    Q_SIGNAL void anyFieldChanged();
};


template<typename StructType>
class StructQObjectWrapper : public StructQObjectWrapperBase
{

public:
    StructQObjectWrapper(QObject *parent = nullptr) : StructQObjectWrapperBase(parent)
    {
    }

    template<typename Type, typename QmlType>
    void assignFromQmlType(facelift::Property<Type> &field, const QmlType &qmlValue)
    {
        Type newFieldValue;
        facelift::assignFromQmlType(newFieldValue, qmlValue);
        field = newFieldValue;
    }

protected:
    StructType m_data;

};

class FaceliftModelLib_EXPORT QMLImplListPropertyBase : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE virtual int size() const = 0;

    Q_PROPERTY(QList<QVariant> content READ elementsAsVariant WRITE setElementsAsVariant NOTIFY elementsChanged)

    Q_SIGNAL void elementsChanged();

    virtual QList<QVariant> elementsAsVariant() const = 0;
    virtual void setElementsAsVariant(const QList<QVariant> &list) = 0;

};

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


class FaceliftModelLib_EXPORT QMLImplMapPropertyBase : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE virtual int size() const = 0;

    Q_PROPERTY(QVariantMap content READ elementsAsVariant WRITE setElementsAsVariant NOTIFY elementsChanged)

    Q_SIGNAL void elementsChanged();

    virtual QVariantMap elementsAsVariant() const = 0;
    virtual void setElementsAsVariant(const QVariantMap &map) = 0;

};

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



template<typename ElementType>
class QMLImplListProperty : public TQMLImplListProperty<ElementType>
{

};


template<typename ElementType>
class QMLImplMapProperty : public TQMLImplMapProperty<ElementType>
{

};

}
