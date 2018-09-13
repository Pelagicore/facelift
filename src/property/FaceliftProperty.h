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

#if defined(FaceliftPropertyLib_LIBRARY)
#  define FaceliftPropertyLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftPropertyLib_EXPORT Q_DECL_IMPORT
#endif

#include <vector>
#include <assert.h>
#include <QString>
#include <QObject>
#include <QDebug>
#include <QTimer>
#include <functional>

#include "FaceliftModel.h"

namespace facelift {

class FaceliftPropertyLib_EXPORT PropertyBase
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
    void init(QObject *ownerObject, void (ServiceType::*changeSignal)(), void (ServiceType::*readySignal)(),
            const char *name = "Unknown")
    {
        init(ownerObject, changeSignal, name);
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

    Property &bind(const GetterFunction &f)
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

/**
 * Specialization used to store a reference to an interface.
 */
template<typename Type>
class ServiceProperty : public Property<Type *>
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

        Property<Type *>::setValue(newValue);
    }

    bool isDirty() const override
    {
        return ((this->m_previousValue != this->value()) || m_valueChanged);
    }

    void clean() override
    {
        Property<Type *>::clean();
        m_valueChanged = false;
    }

    using Property<Type *>::bind;

private:

    bool m_valueChanged = false;

    QPointer<Type> m_pointer;
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
class MapProperty : public Property<QMap<QString, ElementType> >
{

public:
    using Property<QMap<QString, ElementType> >::operator=;

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
        QObject::connect(this, &facelift::ModelBase::beginInsertElements, this, [this] (int first, int last) {
            m_size += last - first + 1;
        });
        QObject::connect(this, &facelift::ModelBase::beginRemoveElements, this, [this] (int first, int last) {
            m_size -= last - first + 1;
        });
        QObject::connect(this, &facelift::ModelBase::beginResetModel, this, [this] () {
            m_resettingModel = true;
        });
        QObject::connect(this, &facelift::ModelBase::endResetModel, this, [this] () {
            m_resettingModel = false;
        });
    }

    template<typename Class>
    ModelProperty &bind(const ModelPropertyInterface<Class, ElementType> &property)
    {
        facelift::Model<ElementType>* modelProperty = property.property;
        QObject::connect(modelProperty, &facelift::ModelBase::beginInsertElements, this, &facelift::ModelBase::beginInsertElements);
        QObject::connect(modelProperty, &facelift::ModelBase::endInsertElements, this, &facelift::ModelBase::endInsertElements);
        QObject::connect(modelProperty, &facelift::ModelBase::beginRemoveElements, this, &facelift::ModelBase::beginRemoveElements);
        QObject::connect(modelProperty, &facelift::ModelBase::endRemoveElements, this, &facelift::ModelBase::endRemoveElements);
        QObject::connect(modelProperty, &facelift::ModelBase::beginResetModel, this, &facelift::ModelBase::beginResetModel);
        QObject::connect(modelProperty, &facelift::ModelBase::endResetModel, this, [this, modelProperty] () {
            m_size = modelProperty->size();
        });
        QObject::connect(modelProperty, &facelift::ModelBase::endResetModel, this, &facelift::ModelBase::endResetModel);
        QObject::connect(modelProperty, static_cast<void (facelift::ModelBase::*)(int,int)>(&facelift::ModelBase::dataChanged),
                (facelift::ModelBase*)this, static_cast<void (facelift::ModelBase::*)(int,int)>(&facelift::ModelBase::dataChanged));

        this->beginResetModel();
        this->reset(property.property->size(), [modelProperty](int index) {
            return modelProperty->elementAt(index);
        });
        this->endResetModel();
        return *this;
    }

    ElementType elementAt(int index) const override
    {
        Q_ASSERT(m_elementGetter);
        Q_ASSERT(!m_resettingModel);
        Q_ASSERT(index < size());
        return m_elementGetter(index);
    }

    void reset(int size, ElementGetter getter)
    {
        m_size = size;
        m_elementGetter = getter;
        notifyDataChanged();
    }

    void notifyDataChanged()
    {
        m_modified = true;
        triggerValueChangedSignal();
    }

    int size() const override
    {
        return m_size;
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
    int m_size = 0;
    bool m_resettingModel = false;
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
