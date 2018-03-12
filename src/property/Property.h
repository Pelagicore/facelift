/*
 *   This is part of the FaceLift project
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

#include "FaceliftModel.h"

namespace facelift {

class PropertyBase
{

public:
    typedef void (QObject::*ChangeSignal)();

    PropertyBase();

    virtual ~PropertyBase();

    template<typename ServiceType>
    void init(const char *name, QObject *ownerObject, void (ServiceType::*changeSignal)())
    {
        m_ownerObject = ownerObject;
        m_ownerSignal = static_cast<ChangeSignal>(changeSignal);
        m_name = name;
    }

    void triggerValueChangedSignal();

    QObject *owner() const
    {
        return m_ownerObject;
    }

    const char *name() const
    {
        return m_name;
    }

    ChangeSignal signalPointer() const
    {
        return m_ownerSignal;
    }

protected:
    virtual void clean() = 0;

    virtual bool isDirty() const = 0;

    virtual QString toString() const = 0;

    void doBreakBinding();

    void setGetterFunctionContext(QObject *context);

private:
    void doTriggerChangeSignal();

    QObject *m_ownerObject = nullptr;
    ChangeSignal m_ownerSignal = nullptr;

    const char *m_name = nullptr;
    bool m_notificationTimerEnabled = false;
    bool m_asynchronousNotification = false;

protected:
    QObject *m_getterFunctionContext = nullptr;
    QMetaObject::Connection m_getterFunctionContextConnection;
    QVector<QMetaObject::Connection> m_connections;  /// The list of connections which this property is bound to

};

/**
 * This template helper class wraps a value and a "value changed" signal which should be triggered whenever the value is changed.
 * When changing the value of the property, the user of this class does not need to explicitly trigger the signal since this is
 * done automatically for him.
 */
template<typename Type>
class Property : public PropertyBase
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

    QString toString() const override
    {
        return facelift::toString(value());
    }

    template<typename Class, typename PropertyType>
    Property &bind(const PropertyInterface<Class, PropertyType> &property)
    {
        this->bind([property] () {
                return property.value();
            }).addTrigger(property.object, property.signal);
        return *this;
    }

    /**
     * Add the given property to the properties which "this" property is bound to, which means that the value of "this" property will
     * be reevaluated whenever the signal is triggered
     */
    template<typename Class, typename PropertyType>
    Property &addTrigger(const PropertyInterface<Class, PropertyType> &property)
    {
        this->addTrigger(property.object, property.signal);
        return *this;
    }

    /**
     * Add the given signal to the signals which this property is bound to, which means that the value of the property will
     * be reevaluated whenever the signal is triggered
     */
    template<typename SourceObjectType, typename SourceSignalType, typename ... Args>
    Property &addTrigger(const SourceObjectType *source, void (SourceSignalType::*changeSignal)(Args ...))
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

    operator const Type &() const {
        return value();
    }

    Property &bind(QObject *context, const GetterFunction &f)
    {
        breakBinding();

        setGetterFunctionContext(context);
        m_getterFunction = f;
        reevaluate();

        return *this;
    }

    Property &bind(const GetterFunction &f)
    {
        return bind(nullptr, f);
    }

    void setValue(const Type &right)
    {
        breakBinding();

        m_value = right;
        qDebug() << "Written value to property " << name() << ":" << toString();
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

    void clean() override
    {
        m_previousValue = m_value;
        //        qDebug() << "Cleaning " << name() << " value: " << m_previousValue;
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
    void breakBinding()
    {
        if (m_getterFunction) {
            m_getterFunction = nullptr;
            doBreakBinding();
        }
    }

    Type m_value = {};  /// The current value

private:
    void reevaluate()
    {
        Q_ASSERT(m_getterFunction);
        m_value = m_getterFunction();
        triggerValueChangedSignal();
    }

    Type m_previousValue = m_value;  /// The value when the last "value changed" signal was triggered

    GetterFunction m_getterFunction;  /// The bound getter function, if any

};


template<typename Type>
class ServiceProperty : public Property<Type *>
{

public:
    template<typename Class, typename PropertyType>
    ServiceProperty &bind(const ServicePropertyInterface<Class, PropertyType> &property)
    {
        Q_UNUSED(property);
        // TODO : implement
        return *this;
    }

    using Property<Type *>::operator=;

};

template<typename ElementType>
class ListProperty : public Property<QList<ElementType> >
{

public:
    using Property<QList<ElementType> >::operator=;

    void removeElementById(ModelElementID elementId)
    {
        bool bModified = false;
        for (int i = 0; i < size(); i++) {
            if (this->value()[i].id() == elementId) {
                this->modifiableValue().removeAt(i);
                bModified = true;
                break;
            }
        }

        if (bModified) {
            this->triggerValueChangedSignal();
        }
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

    const ElementType *elementPointerById(ModelElementID id) const
    {
        for (const auto &element : this->value()) {
            if (element.id() == id) {
                return &element;
            }
        }
        return nullptr;
    }

private:
    QList<ElementType> &modifiableValue()
    {
        this->breakBinding();
        return this->m_value;
    }

};


template<typename ElementType>
class ModelProperty : public PropertyBase
{
public:
    typedef std::function<ElementType(int)> ElementGetter;

    ModelProperty()
    {
    }

    template<typename Class, typename PropertyType>
    ModelProperty &bind(const ModelPropertyInterface<Class, PropertyType> &property)
    {
        Q_UNUSED(property);
        // TODO : implement
        return *this;
    }

    ElementType elementAt(int index) const
    {
        if (m_elementGetter != nullptr) {
            return m_elementGetter(index);
        } else {
            return m_elements[index];
        }
    }

    void setGetter(ElementGetter getter)
    {
        m_elementGetter = getter;
    }

    void setSize(size_t size)
    {
        if (m_size != size) {
            m_size = size;
            notifyDataChanged();
        }
    }

    void notifyDataChanged()
    {
        m_modified = true;
        triggerValueChangedSignal();
    }

    size_t size() const
    {
        if (m_elementGetter != nullptr) {
            return m_size;
        } else {
            return m_elements.size();
        }
    }

    bool isDirty() const override
    {
        return m_modified;
    }

    void clean() override
    {
        m_modified = false;
    }

    void setElements(const QList<ElementType> &elements)
    {
        m_elements = elements;
        notifyDataChanged();
    }

    QList<ElementType> asList() const
    {
        QList<ElementType> list;
        auto elementCount = size();
        for (size_t i = 0; i < elementCount; i++) {
            list.append(elementAt(i));
        }
        return list;
    }

    QString toString() const override
    {
        NOT_IMPLEMENTED();
        return "Model";
    }

private:
    ElementGetter m_elementGetter;
    QList<ElementType> m_elements;
    size_t m_size = 0;

    bool m_modified = false;
};


template<typename InterfaceType>
struct PropertyConnector
{
    typedef void (InterfaceType::*ChangeSignal)();

    template<typename Param1, typename Param2>
    static QMetaObject::Connection connect(const PropertyBase &property, Param1 *context, const Param2 &p2)
    {
        auto signal = static_cast<ChangeSignal>(property.signalPointer());
        auto source = static_cast<InterfaceType *>(property.owner());
        return QObject::connect(source, signal, context, p2);
    }

};

}
