#pragma once

#include "Model.h"
#include "Property.h"

namespace facelift {

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

    static void fromJSValue(Type &v, const QJSValue &value, QQmlEngine *engine)
    {
        v = engine->fromScriptValue<Type>(value);
    }

};


template<typename StructType>
struct QMLModelTypeHandler<StructType, typename std::enable_if<std::is_base_of<StructureBase, StructType>::value>::type>
{
    static QJSValue toJSValue(const StructType &f, QQmlEngine *engine)
    {
        return structToJSValue(f, engine);
    }

    static void fromJSValue(StructType &v, const QJSValue &value, QQmlEngine *engine)
    {
        v = engine->fromScriptValue<StructType>(value);
    }

};


template<typename EnumType>
struct QMLModelTypeHandler<EnumType, typename std::enable_if<std::is_enum<EnumType>::value>::type>
{
    static QJSValue toJSValue(const EnumType &v, QQmlEngine *engine)
    {
        return enumToJSValue(v, engine);
    }

    static void fromJSValue(EnumType &v, const QJSValue &value, QQmlEngine *engine)
    {
        v = engine->fromScriptValue<EnumType>(value);
    }

};


template<typename ListElementType>
struct QMLModelTypeHandler<QList<ListElementType> >
{
    typedef QList<ListElementType> Type;
    static QJSValue toJSValue(const Type &v, QQmlEngine *engine)
    {
        return engine->toScriptValue(v);
    }

    static void fromJSValue(Type &v, const QJSValue &value, QQmlEngine *engine)
    {
        v = engine->fromScriptValue<Type>(value);
    }

};

template<typename Type>
QJSValue toJSValue(const Type &v, QQmlEngine *engine)
{
    return QMLModelTypeHandler<Type>::toJSValue(v, engine);
}


template<typename Type>
void fromJSValue(Type &v, const QJSValue &jsValue, QQmlEngine *engine)
{
    QMLModelTypeHandler<Type>::fromJSValue(v, jsValue, engine);
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

    QMLModelImplementationType *m_impl = nullptr;

};


class QMLImplListPropertyBase :
    public QObject
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
class TQMLImplListProperty :
    public QMLImplListPropertyBase
{
public:
    Property<QList<ElementType> > &property() const
    {
        Q_ASSERT(m_property != nullptr);
        return *m_property;
    }

    QList<QVariant> elementsAsVariant() const override
    {
        return toQMLCompatibleType(elements());
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

    void setProperty(Property<QList<ElementType> > &property)
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

    /*
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

        ElementType removeElementByID(ModelElementID elementId)
        {
            ElementType returnValue;
            auto list = m_property->value();
            for (const auto &element : list) {
                if (element.id() == elementId) {
                    returnValue = element;
                    list.removeAll(element);
                    m_property->setValue(list);
                    break;
                }
            }
            return returnValue;
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
    */

private:
    Property<QList<ElementType> > *m_property = nullptr;
    QList<QVariant> m_assignedVariantList;
    QList<QMetaObject::Connection> m_changeSignalConnections;
};


template<typename ElementType>
class QMLImplListProperty :
    public TQMLImplListProperty<ElementType>
{

};


class StructQObjectWrapperBase :
    public QObject
{

    Q_OBJECT

public:
    StructQObjectWrapperBase(QObject *parent = nullptr) : QObject(parent)
    {
        m_id.init("id", this, &StructQObjectWrapperBase::idChanged);
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
    }

protected:
    Property<int> m_id;

};


template<typename StructType>
class StructQObjectWrapper :
    public StructQObjectWrapperBase
{

public:
    StructQObjectWrapper(QObject *parent = nullptr) : StructQObjectWrapperBase(parent)
    {
    }

protected:
    StructType m_data;

};

class QObjectWrapperPointerBase
{

public:
    void addConnection(QMetaObject::Connection connection)
    {
        m_connections.append(connection);
    }

    void disconnect()
    {
        for (const auto &connection : m_connections) {
            auto successfull = QObject::disconnect(connection);
            Q_ASSERT(successfull);
        }
        m_connections.clear();
    }

private:
    QList<QMetaObject::Connection> m_connections;

};

template<typename StructQMLWrapperType>
class QObjectWrapperPointer :
    public QObjectWrapperPointerBase
{

public:
    bool isSet() const
    {
        return !m_pointer.isNull();
    }

    void reset(StructQMLWrapperType *p)
    {
        disconnect();
        m_pointer = p;
    }

    void clear()
    {
        disconnect();
        m_pointer = nullptr;
    }

    StructQMLWrapperType *object() const
    {
        return m_pointer.data();
    }

private:
    QPointer<StructQMLWrapperType> m_pointer;

};


}
