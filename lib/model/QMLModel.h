#pragma once

#include "Model.h"
#include "property/Property.h"

class QMLModelImplementationFrontendBase
{
protected:
    QQmlEngine *qmlEngine()
    {
        if (s_engine == nullptr) {
            s_engine = new QQmlEngine();
        }
        return s_engine;
    }

    static QQmlEngine *s_engine;
};

template<typename QMLModelImplementationType>
class QMLModelImplementationFrontend :
    public QMLModelImplementationFrontendBase
{
protected:
    QMLModelImplementationFrontend()
    {
    }

    QMLModelImplementationType *m_impl;

};


class QMLImplListPropertyBase :
    public QObject
{

    Q_OBJECT

public:
    Q_INVOKABLE virtual int size() const = 0;

    Q_PROPERTY(QList<QVariant> elements READ elementsAsVariant NOTIFY elementsChanged)

    Q_SIGNAL void elementsChanged();

    virtual QList<QVariant> elementsAsVariant() const = 0;

};

template<typename ElementType>
class QMLImplListProperty :
    public QMLImplListPropertyBase
{
public:
    Property<QList<ElementType> > &property() const
    {
        Q_ASSERT(m_property != nullptr);
        return *m_property;
    }

    // TODO : check why the QML engine does not seem to be able to handle the return type of this method
    QList<QVariant> elementsAsVariant() const {
    	return toQMLCompatibleType(elements());
    }

    void setProperty(Property<QList<ElementType> > &property)
    {
        m_property = &property;
    }

    int size() const override
    {
        return property().value().size();
    }

    const QList<ElementType>& elements() const
    {
        return property().value();
    }

    void addElement(ElementType element)
    {
        auto list = m_property->value();
        list.append(element);
        m_property->setValue(list);
    }

    bool elementExists(ModelElementID elementId) const
    {
        for (const auto &element : property().value()) {
            if (element.id() == elementId) {
                return true;
            }
        }
        return false;
    }

    const ElementType *elementById(ModelElementID elementId) const
    {
        Q_ASSERT(elementExists(elementId));
        for (const auto &element : property().value()) {
            if (element.id() == elementId) {
                return &element;
            }
        }
        return nullptr;
    }

    int elementIndexById(ModelElementID elementId) const
    {
        auto &list = property().value();
        for (int index = 0; index < list.size(); index++) {
            if (list[index].id() == elementId) {
                return index;
            }
        }
        return -1;
    }

    const ElementType &elementAt(int index) const
    {
        Q_ASSERT((index >= 0) && (index < size()));
        return property().value()[index];
    }

private:
    Property<QList<ElementType> > *m_property = nullptr;

};


typedef QMLImplListProperty<bool> boolQMLImplListProperty;
typedef QMLImplListProperty<int> intQMLImplListProperty;
typedef QMLImplListProperty<QString> stringQMLImplListProperty;
