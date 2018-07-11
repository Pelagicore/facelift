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

#include "FaceliftModel.h"
#include "QMLFrontend.h"
#include "FaceliftProperty.h"

namespace facelift {


/**
 * Base class for
 */
class ModelQMLImplementationBase : public QObject, public QQmlParserStatus
{
    Q_OBJECT

public:
    // We set a default property so that we can have children QML elements in our QML implementations, such as Timer
    Q_PROPERTY(QQmlListProperty<QObject> childItems READ childItems)
    Q_CLASSINFO("DefaultProperty", "childItems")

    ModelQMLImplementationBase(QObject *parent = nullptr) :
        QObject(parent)
    {
    }

    Q_PROPERTY(bool ready READ ready WRITE setReady NOTIFY readyChanged)

    void setReady(bool ready)
    {
        Q_ASSERT(m_interface != nullptr);
        m_interface->setReady(ready);
    }

    bool ready() const
    {
        Q_ASSERT(m_interface != nullptr);
        return m_interface->ready();
    }

    Q_SIGNAL void readyChanged();

    Q_PROPERTY(QString implementationID READ implementationID WRITE setImplementationID NOTIFY implementationIDChanged)

    void setImplementationID(const QString &id)
    {
        m_implementationID = id;
        assignImplementationID();
    }

    const QString &implementationID() const
    {
        return m_implementationID;
        Q_ASSERT(m_interface != nullptr);
        return m_interface->implementationID();
    }

    Q_SIGNAL void implementationIDChanged();

    void setInterface(InterfaceBase *i)
    {
        m_interface = i;
        QObject::connect(i, &InterfaceBase::readyChanged, this, &ModelQMLImplementationBase::readyChanged);

        assignImplementationID();
        QObject::connect(this, &QObject::objectNameChanged, this, &ModelQMLImplementationBase::assignImplementationID);
    }

    QJSValue &checkMethod(QJSValue &method, const char *methodName)
    {
        if (!method.isCallable()) {
            qFatal("Method \"%s\" of Facelift interface implementation \"%s\" is about to be called but it is not implemented in your QML file. "
                    "That method MUST be implemented if it is called.", qPrintable(methodName), qPrintable(interfac()->interfaceID()));
        }

        return method;
    }

    QQmlListProperty<QObject> childItems()
    {
        return QQmlListProperty<QObject>(this, m_children);
    }

    void classBegin() override
    {
    }

    void componentComplete() override
    {
        assignImplementationID();
        m_interface->componentCompleted();
    }

    facelift::InterfaceBase *interfac() const
    {
        return m_interface;
    }

    void assignImplementationID()
    {
        Q_ASSERT(m_interface != nullptr);
        QString id;
        QTextStream s(&id);

        s << "QML implementation - " << metaObject()->className();
        if (!objectName().isEmpty()) {
            s << ", name = " << objectName();
        }
        s << "" << this;

        m_interface->setImplementationID(id);
    }

private:
    QList<QObject *> m_children;
    InterfaceBase *m_interface = nullptr;
    QString m_implementationID;
};

template<typename InterfaceType>
class ModelQMLImplementation : public ModelQMLImplementationBase
{

public:
    ModelQMLImplementation(QObject *parent = nullptr) :
        ModelQMLImplementationBase(parent)
    {
    }

    static void setModelImplementationFilePath(QString path)
    {
        modelImplementationFilePath() = path;
    }

    static QString &modelImplementationFilePath()
    {
        static QString s_modelImplementationFilePath;
        return s_modelImplementationFilePath;
    }

    virtual void initProvider(InterfaceType *provider) = 0;

    void retrieveFrontend()
    {
        m_interface = retrieveFrontendUnderConstruction();
        if (m_interface == nullptr) {
            m_interface = createFrontend();
        }

        Q_ASSERT(m_interface);

        initProvider(m_interface);
        setInterface(m_interface);
    }

    virtual InterfaceType *createFrontend() = 0;

    static void registerTypes(const char *theURI)
    {
        typedef typename InterfaceType::QMLImplementationModelType QMLImplementationModelType;

        // Register the component used to actually implement the model in QML
        // the QML file containing the model implementation should have this type at its root
        ::qmlRegisterType<QMLImplementationModelType>(theURI, 1, 0, QMLImplementationModelType::QML_NAME);
    }

    static InterfaceType *retrieveFrontendUnderConstruction()
    {
        auto instance = frontendUnderConstruction();
        frontendUnderConstruction() = nullptr;
        return instance;
    }

    static void setFrontendUnderConstruction(InterfaceType *instance)
    {
        Q_ASSERT(frontendUnderConstruction() == nullptr);
        frontendUnderConstruction() = instance;
    }

    void checkInterface() const
    {
        Q_ASSERT(m_interface != nullptr);
    }

private:
    static InterfaceType * &frontendUnderConstruction()
    {
        static InterfaceType *i = nullptr;
        return i;
    }

protected:
    InterfaceType *provider() const
    {
        return m_interface;
    }

    InterfaceType *m_interface = nullptr;

};


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


class QMLImplMapPropertyBase : public QObject
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
class QMLImplMapProperty : public TQMLImplMapProperty<ElementType>
{

};


class StructQObjectWrapperBase : public QObject
{
    Q_OBJECT

public:
    StructQObjectWrapperBase(QObject *parent = nullptr) : QObject(parent)
    {
        m_id.init(this, &StructQObjectWrapperBase::idChanged, "id");
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
    return ::qmlRegisterType<TQMLFrontend<typename QMLImplementationType::Provider> >(uri, majorVersion, minorVersion, componentName);
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
