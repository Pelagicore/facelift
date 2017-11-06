#pragma once

#include "Model.h"
#include "QMLFrontend.h"
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
class QMLModelImplementationFrontend : public QMLModelImplementationFrontendBase
{

protected:
    QMLModelImplementationFrontend()
    {
    }

    template<typename ModelImplClass, typename InterfaceType>
    ModelImplClass *createComponent(QQmlEngine *engine, InterfaceType *frontend)
    {
        auto path = ModelImplClass::modelImplementationFilePath();
        // Save the reference to the frontend which we are currently creating, so that the QML model implementation is able
        // to access it from its constructor
        ModelImplClass::setFrontendUnderConstruction(frontend);
        ModelImplClass *r = nullptr;
        qDebug() << "Creating QML component from file : " << path;
        QQmlComponent component(engine, QUrl::fromLocalFile(path));
        if (!component.isError()) {
            QObject *object = component.create();
            r = qobject_cast<ModelImplClass *>(object);
        } else {
            qWarning() << "Error : " << component.errorString();
            qFatal("Can't create QML model");
        }
        return r;
    }

    QMLModelImplementationType *m_impl = nullptr;

};


class QMLImplListPropertyBase : public QObject
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

private:
    Property<QList<ElementType> > *m_property = nullptr;
    QList<QVariant> m_assignedVariantList;
    QList<QMetaObject::Connection> m_changeSignalConnections;
};


template<typename ElementType>
class QMLImplListProperty : public TQMLImplListProperty<ElementType>
{

};


class StructQObjectWrapperBase : public QObject
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
class StructQObjectWrapper : public StructQObjectWrapperBase
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
class QObjectWrapperPointer : public QObjectWrapperPointerBase
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

/*!
 * Register the given interface QML implementation as a creatable QML component.
 * By default, the component is registered under the same name as defined in the Qface definition.
 * \param qmlFilePath Path of the file containing the QML implementation of the interface
 */
template<typename QMLImplementationType>
int registerQmlComponent(const char *uri, const char *qmlFilePath,
        const char *componentName = QMLImplementationType::Provider::QMLFrontendType::INTERFACE_NAME,
        int majorVersion = QMLImplementationType::Provider::VERSION_MAJOR,
        int minorVersion = QMLImplementationType::Provider::VERSION_MINOR,
        typename std::enable_if<std::is_base_of<facelift::ModelQMLImplementationBase, QMLImplementationType>::value>::type * = nullptr)
{
//    qDebug() << "Registering QML implementation \"" << qmlFilePath << "\" for component \"" << componentName << "\"";
    QMLImplementationType::Provider::registerTypes(uri);
    QMLImplementationType::setModelImplementationFilePath(qmlFilePath);
    return ::qmlRegisterType<TQMLFrontend<typename QMLImplementationType::Provider>>(uri, majorVersion, minorVersion, componentName);
}

/*!
 * Register the given interface QML implementation as QML singleton.
 * By default, the component is registered under the same name as defined in the Qface definition.
 * \param qmlFilePath Path of the file containing the QML implementation of the interface
 */
template<typename QMLImplementationType>
int registerSingletonQmlComponent(const char *uri, const char *qmlFilePath,
        const char *name = QMLImplementationType::Provider::QMLFrontendType::INTERFACE_NAME,
        int majorVersion = QMLImplementationType::Provider::VERSION_MAJOR,
        int minorVersion = QMLImplementationType::Provider::VERSION_MINOR,
        typename std::enable_if<std::is_base_of<facelift::ModelQMLImplementationBase, QMLImplementationType>::value>::type * = nullptr)
{
    QMLImplementationType::Provider::registerTypes(uri);
    QMLImplementationType::setModelImplementationFilePath(qmlFilePath);
    typedef TQMLFrontend<typename QMLImplementationType::Provider> QMLType;
    return ::qmlRegisterSingletonType<QMLType>(uri, majorVersion, minorVersion, name, &singletonGetter<QMLType>);
}


}
