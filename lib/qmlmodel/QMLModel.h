#pragma once

#include "Model.h"
#include "Property.h"


template<typename EnumType>
inline QJSValue enumToJSValue(const EnumType e, QQmlEngine *engine)
{
    Q_UNUSED(engine);
    return static_cast<int>(e);
}

template<typename StructType>
inline QJSValue structToJSValue(const StructType &s, QQmlEngine *engine)
{
    return engine->toScriptValue(s);
}


template<typename Type, typename Sfinae = void>
struct QMLModelTypeHandler
{
    static QJSValue toJSValue(const Type &v, QQmlEngine *engine)
    {
        return engine->toScriptValue(v);
    }
};

template<typename StructType>
struct QMLModelTypeHandler<StructType, typename std::enable_if<std::is_base_of<ModelStructure, StructType>::value>::type>
{
    static QJSValue toJSValue(const StructType &f, QQmlEngine *engine)
    {
        return structToJSValue(f, engine);
    }
};


template<typename EnumType>
struct QMLModelTypeHandler<EnumType, typename std::enable_if<std::is_enum<EnumType>::value>::type>
{
    static QJSValue toJSValue(const EnumType &v, QQmlEngine *engine)
    {
        return enumToJSValue(v, engine);
    }
};


template<typename ListElementType>
struct QMLModelTypeHandler<QList<ListElementType> >
{
    static QJSValue toJSValue(const QList<ListElementType> &v, QQmlEngine *engine)
    {
        Q_ASSERT(false);
        return enumToJSValue(v, engine);
    }
};

template<typename Type>
QJSValue toJSValue(const Type &v, QQmlEngine *engine)
{
    return QMLModelTypeHandler<Type>::toJSValue(v, engine);
}


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
class TQMLImplListProperty :
    public QMLImplListPropertyBase
{
public:
    Property<QList<ElementType> > &property() const
    {
        Q_ASSERT(m_property != nullptr);
        return *m_property;
    }

    // TODO : check why the QML engine does not seem to be able to handle the return type of this method
    QList<QVariant> elementsAsVariant() const override
    {
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

    const QList<ElementType> &elements() const
    {
        return property().value();
    }

    ElementType addElement(ElementType element)
    {
        auto list = m_property->value();
        list.append(element);
        m_property->setValue(list);
        return list[list.size() - 1];
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

    void removeElementByID(ModelElementID elementId)
    {
        auto list = m_property->value();
        for (const auto &element : list) {
            if (element.id() == elementId) {
                list.removeAll(element);
                m_property->setValue(list);
                break;
            }
        }
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

template<typename ElementType>
class QMLImplListProperty :
    public TQMLImplListProperty<ElementType>
{

};


class StructQMLWrapperBase :
    public QObject
{

    Q_OBJECT

public:
    StructQMLWrapperBase(QObject *parent = nullptr) : QObject(parent)
    {
        m_id.init("id", this, &StructQMLWrapperBase::idChanged);
    }

    Q_PROPERTY(int uid READ id WRITE setId NOTIFY idChanged)

    Q_SIGNAL void idChanged();

    ModelElementID id() const
    {
        return m_id.value();
    }

    void setId(int id)
    {
        m_id = id;
        ;
    }

    Q_SIGNAL void onAnyFieldChanged();

protected:
    Property<int> m_id;

};

template<typename StructType>
class StructQMLWrapper :
    public StructQMLWrapperBase
{

public:
    StructQMLWrapper(QObject *parent = nullptr) : StructQMLWrapperBase(parent)
    {
    }

protected:
    StructType m_data;

};
