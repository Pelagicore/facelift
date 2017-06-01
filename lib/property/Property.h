/*
 *   This is part of the QMLCppAPI project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 *   Please see the LICENSE file for details.
 */

#pragma once

#include <vector>
#include <assert.h>
#include <QString>
#include <QObject>
#include <QDebug>
#include <QTimer>
#include <functional>

#include "model/Model.h"


class PropertyBase
{

public:
    typedef void (QObject::*ChangeSignal)();

    PropertyBase()
    {
    }

    void doTriggerChangeSignal()
    {
        if (signal() == nullptr) {
            qFatal("init() has not been called");
        }

        (m_ownerObject->*m_ownerSignal)();
        clean();
    }

    void triggerValueChangedSignal()
    {
        if (!m_timerEnabled) {
            m_timerEnabled = true;

            QTimer::singleShot(0, m_ownerObject, [this] () {
                        if (isValueChanged()) {
                            qDebug() << "Property " << m_name << " : Triggering notification";
                            doTriggerChangeSignal();
                        } else {
                            //                            qDebug() << "Property " << m_name << " : Triggering notification. value unchanged: " << toString();
                        }
                        m_timerEnabled = false;
                    });

        }
    }

    template<typename ServiceType>
    void init(const char *name, QObject *ownerObject, void (ServiceType::*changeSignal)())
    {
        m_ownerObject = ownerObject;
        m_ownerSignal = (ChangeSignal) changeSignal;
        m_name = name;
    }

    virtual QString toString() const
    {
        Q_ASSERT(false);
    }

    virtual bool isValueChanged() const = 0;

    virtual void clean() = 0;

    QObject *owner() const
    {
        return m_ownerObject;
    }

    ChangeSignal signal() const
    {
        return m_ownerSignal;
    }

    const char *name() const
    {
        return m_name;
    }

private:
    QObject *m_ownerObject = nullptr;
    ChangeSignal m_ownerSignal = nullptr;

    bool m_timerEnabled = false;
    const char *m_name = nullptr;

};

/**
 * This template helper class wraps a value and a "value changed" signal which should be triggered whenever the value is changed.
 * When changing the value of the property, the user of this class does not need to explicitly trigger the signal since this is
 * done automatically for him.
 */
template<typename Type>
class Property :
    public PropertyBase
{

    typedef std::function<Type()> GetterFunction;

public:
    Property(Type initialValue)
    {
        m_value = m_previousValue = initialValue;
    }

    Property()
    {
    }

    template<typename Class, typename PropertyType>
    Property &bind(const PropertyInterface<Class, PropertyType> &property)
    {
        breakBinding();
        m_getterFunction = [property] () {
            return property.value();
        };
        addDependency(property);
        reevaluate();

        return *this;
    }

    /**
     * Add the given property to the properties which "this" property is bound to, which means that the value of "this" property will
     * be reevaluated whenever the signal is triggered
     */
    template<typename Class, typename PropertyType>
    Property &connect(const PropertyInterface<Class, PropertyType> &property)
    {
        this->connect(property.object, property.signal);
        return *this;
    }

    /**
     * Add the given signal to the signals which this property is bound to, which means that the value of the property will
     * be reevaluated whenever the signal is triggered
     */
    template<typename SourceType, typename ... Args>
    Property &connect(SourceType *source, void (SourceType::*changeSignal)(Args ...))
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

    Type &modifiableValue()
    {
        if (m_getterFunction) {
            qDebug() << name() << " property : breaking binding";
            breakBinding();
        }

        // We return a modifiable reference so we might have to trigger a "value changed" signal later
        triggerValueChangedSignal();
        return m_value;
    }

    operator const Type() const {
        return value();
    }

    Property &bind(const GetterFunction &f)
    {
        if (m_getterFunction) {
            qDebug() << name() << " property : breaking binding";
            breakBinding();
        }

        m_getterFunction = f;
        reevaluate();

        return *this;
    }

    void setValue(const Type &right)
    {
        if (m_getterFunction) {
            qDebug() << name() << " property : breaking binding";
            breakBinding();
            m_getterFunction = nullptr;
        }

        m_value = right;
        qDebug() << "Written value to property " << name();
        triggerValueChangedSignal();
    }

    Type &operator=(const Type &right)
    {
        setValue(right);
        return m_value;
    }

    bool isValueChanged() const override
    {
        return !(m_previousValue == m_value);
    }

    void clean() override
    {
        m_previousValue = m_value;
        //        qDebug() << "Cleaning " << name() << " value: " << m_previousValue;
    }

    Type operator-=(const Type &right)
    {
        return operator=(value() - right);
    }

    Type operator+=(const Type &right)
    {
        return operator=(value() + right);
    }

    Type operator*=(const Type &right)
    {
        return operator=(value() * right);
    }

    Type operator/=(const Type &right)
    {
        return operator=(value() / right);
    }

private:
    void reevaluate()
    {
        Q_ASSERT(m_getterFunction);
        m_value = m_getterFunction();
        triggerValueChangedSignal();
    }

    void breakBinding()
    {
        for (const auto &connection : m_connections) {
            auto successfull = QObject::disconnect(connection);
            Q_ASSERT(successfull);
        }
        m_connections.clear();
    }

    Type m_value = {};  /// The current value
    Type m_previousValue = m_value;  /// The value when the last "value changed" signal was triggered

    GetterFunction m_getterFunction;  /// The bound getter function, if any

    QVector<QMetaObject::Connection> m_connections;  /// The list of connections which this property is bound to

};


template<typename ElementType>
class StructListProperty :
    public Property<QList<ElementType> >
{

    class TheModelListModel :
        public ModelListModel
    {

public:
        TheModelListModel(StructListProperty &listProperty) :
            m_listProperty(listProperty)
        {
        }

        QHash<int, QByteArray> roleNames() const override
        {
            return ElementType::roleNames_(ElementType::FIELD_NAMES);
        }

        int rowCount(const QModelIndex &index) const override
        {
            Q_UNUSED(index);
            return list().size();
        }

        QVariant data(const QModelIndex &index, int role) const override
        {
            return list().at(index.row()).getFieldAsVariant(role);
        }

        void beginChange()
        {
            if (!m_changeOnGoing) {

                m_changeOnGoing = true;
                ModelListModel::beginResetModel();

                QTimer::singleShot(0, this, [this] () {
                            ModelListModel::endResetModel();
                            m_changeOnGoing = false;
                            m_listProperty.doTriggerChangeSignal();
                        });

            }
        }

        const QList<ElementType> &list() const
        {
            return m_listProperty.list();
        }

        int elementID(int elementIndex) const override
        {
            Q_ASSERT(elementIndex >= 0);
            Q_ASSERT(elementIndex < m_listProperty.list().size());

            if (!(elementIndex >= 0) && (elementIndex < m_listProperty.list().size())) {
                qWarning() << "Invalid index : " << elementIndex;
                return -1;
            }

            auto &element = list()[elementIndex];
            return element.id();
        }

private:
        StructListProperty &m_listProperty;
    };

public:
    void removeElementById(ModelElementID elementId)
    {
        for (int i = 0; i < size(); i++) {
            if (this->value()[i].id() == elementId) {
                this->modifiableValue().removeAt(i);
                break;
            }
        }
    }

    void addElement(ElementType element)
    {
        this->modifiableValue().append(element);
    }

    int size() const
    {
        return this->value().size();
    }

    ElementType *elementPointerById(ModelElementID elementId)
    {
        for (auto &element : this->modifiableValue()) {
            if (element.id() == elementId) {
                return &element;
            }
        }
        return nullptr;
    }

    const ElementType *elementPointerById(ModelElementID id) const
    {
        for (const auto &element : this->value()) {
            if (element.id() == id) {
                return &element;
            }
        }
        return nullptr;
    }

};


template<typename ElementType>
class SimpleTypeListProperty :
    public StructListProperty<ElementType>
{

};

template<typename EnumType>
class EnumListProperty :
    public SimpleTypeListProperty<EnumType>
{
};


typedef SimpleTypeListProperty<int> intListProperty;
typedef SimpleTypeListProperty<QString> stringListProperty;
typedef SimpleTypeListProperty<bool> boolListProperty;
