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

#include "FaceliftCommon.h"
#include "FaceliftModel.h"

namespace facelift {

class FaceliftModelLib_EXPORT PropertyBase
{

public:
    typedef void (QObject::*ChangeSignal)();

    PropertyBase();

    virtual ~PropertyBase();

    template<typename ServiceType>
    void init(QObject *ownerObject, void (ServiceType::*changeSignal)(), const char *name = "Unknown")
    {
        m_ownerObject = ownerObject;
        m_ownerSignal = static_cast<ChangeSignal>(changeSignal);
        m_name = name;
    }

    template<typename ServiceType>
    void setReadyChangedSlot(void (ServiceType::*readySignal)())
    {
        m_readySignal = static_cast<ChangeSignal>(readySignal);
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

    bool &isReady()
    {
        return m_ready;
    }

    void setReady(bool ready)
    {
        if (Q_UNLIKELY(m_readySignal && m_ready != ready)) {
            m_ready = ready;
            (m_ownerObject->*m_readySignal)();
        }
    }

protected:
    virtual void clean() = 0;

    virtual bool isDirty() const = 0;

    virtual QString toString() const = 0;

    void doBreakBinding();

private:
    void doTriggerChangeSignal();

    bool m_ready = true;
    QObject *m_ownerObject = nullptr;
    ChangeSignal m_ownerSignal = nullptr;
    ChangeSignal m_readySignal = nullptr;

    const char *m_name = nullptr;
    bool m_notificationTimerEnabled = false;
    bool m_asynchronousNotification = false;

protected:
    QMetaObject::Connection m_getterFunctionContextConnection;
    QVector<QMetaObject::Connection> m_connections;  /// The list of connections which this property is bound to

};

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
            }).addTrigger(property.object, property.signal);
        return *this;
    }

    /**
     * Add the given property to the properties which "this" property is bound to, which means that the value of "this" property will
     * be reevaluated whenever the signal is triggered
     */
    template<typename Class, typename PropertyType>
    TProperty &addTrigger(const PropertyInterface<Class, PropertyType> &property)
    {
        this->addTrigger(property.object, property.signal);
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

    void clean() override
    {
        m_previousValue = m_value;
        //        qCDebug(LogModel) << "Cleaning " << name() << " value: " << m_previousValue;
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


template<typename Type, typename Enable = void>
class Property : public TProperty<Type>
{
public:
    using TProperty<Type>::operator=;

    Property(Type initialValue) : TProperty<Type>(initialValue)
    {
    }

    Property()
    {
    }

};



/**
 * Specialization used to store a reference to an interface.
 */
template<typename Type>
class ServiceProperty : public TProperty<Type *>
{

public:
    template<typename Class, typename PropertyType>
    ServiceProperty &bind(const ServicePropertyInterface<Class, PropertyType> &property)
    {
        this->bind([property] () {
                return property.value();
            }).addTrigger(property.object, property.signal);
        return *this;
    }

    Type *operator=(Type *right)
    {
        ServiceProperty::setValue(right);
        return this->m_value;
    }

    void setValue(Type *newValue)
    {
        if (newValue != m_pointer) {
            m_valueChanged = true;
            // Store as a QPointer in order to be able to detect object destructions and generate a change signal even if the same
            // pointer is assigned later on
            m_pointer = newValue;
        }

        TProperty<Type *>::setValue(newValue);
    }

    bool isDirty() const override
    {
        return ((this->m_previousValue != this->value()) || m_valueChanged);
    }

    void clean() override
    {
        TProperty<Type *>::clean();
        m_valueChanged = false;
    }

    using TProperty<Type *>::bind;

private:
    bool m_valueChanged = false;

    QPointer<Type> m_pointer;
};



template<typename Type>
class Property<Type *, typename std::enable_if<std::is_base_of<QObject *, Type>::value>::type> : public ServiceProperty<Type>
{
public:
    using ServiceProperty<Type>::operator=;

    Property()
    {
    }

};



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


template<typename ElementType>
class Property<QList<ElementType> > : public ListProperty<ElementType>
{
public:
    using TProperty<QList<ElementType> >::operator=;

};


template<typename ElementType>
class Property<QMap<QString, ElementType> > : public TProperty<QMap<QString, ElementType> >
{

public:
    using TProperty<QMap<QString, ElementType> >::operator=;

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
    QMap<QString, ElementType> &modifiableValue()
    {
        this->breakBinding();
        return this->m_value;
    }

};


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
        facelift::Model<ElementType>* model = property.property;
        this->bindOtherModel(model);

        this->beginResetModel();
        this->reset(property.property->size(), [model](int index) {
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
