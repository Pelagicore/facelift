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

#include <QObject>
#include <QDebug>
#include <QQmlEngine>
#include <QtQml>
#include <functional>
#include <QQmlParserStatus>
#include <QAbstractListModel>

#include "FaceliftModel.h"

namespace facelift {

class IPCAttachedPropertyFactoryBase;
class IPCServiceAdapterBase;


template<typename Type, typename Sfinae = void>
struct QMLModelTypeHandler
{
    static QJSValue toJSValue(const Type &v, QQmlEngine *engine)
    {
        return engine->toScriptValue(facelift::toQMLCompatibleType(v));
    }

    static void fromJSValue(Type &v, const QJSValue &value, QQmlEngine *engine)
    {
        v = engine->fromScriptValue<Type>(value);
    }
};

template<typename Type>
struct QMLModelTypeHandler<Type, typename std::enable_if<std::is_enum<Type>::value>::type>
{
    static QJSValue toJSValue(const Type &v, QQmlEngine *engine)
    {
        Q_UNUSED(engine)
        return QJSValue(v);
    }

    static void fromJSValue(Type &v, const QJSValue &value, QQmlEngine *engine)
    {
        Q_UNUSED(engine)
        v = static_cast<Type>(value.toInt());
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


/*!
 * This is the base class which all QML frontends extend
 */
class FaceliftModelLib_EXPORT QMLFrontendBase : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

public:

    static const QJSValue NO_OPERATION_JS_CALLBACK;

    QMLFrontendBase(QObject *parent);

    /**
     *  This constructor is used when instantiating a singleton
     */
    QMLFrontendBase(QQmlEngine *engine);

    Q_PROPERTY(QObject * provider READ provider CONSTANT)
    virtual InterfaceBase *provider();

    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    bool ready() const;

    Q_SIGNAL void readyChanged();

    Q_PROPERTY(QString implementationID READ implementationID CONSTANT)
    virtual const QString &implementationID();

    void classBegin() override;

    void componentComplete() override;

protected:
    static void appendJSValue(QJSValueList& list, QQmlEngine * engine) {
        Q_UNUSED(list);
        Q_UNUSED(engine);
    }

    template<typename Arg1Type, typename  ... Args>
    static void appendJSValue(QJSValueList& list, QQmlEngine * engine,  const Arg1Type & arg1, const Args & ...args) {
        list.append(facelift::toJSValue(arg1, engine));
        appendJSValue(list, engine, args...);
    }

    template<typename  ... Args>
    void callJSCallback(QJSValue &callback, const Args & ...args)
    {
        if (!callback.isUndefined()) {
            if (callback.isCallable()) {
                QQmlEngine *engine = (m_qmlEngine ? m_qmlEngine : qmlEngine(this));
                Q_ASSERT(engine != nullptr);
                QJSValueList jsList;
                appendJSValue(jsList, engine, args...);
                auto returnValue = callback.call(jsList);
                if (returnValue.isError()) {
                    qCritical("Error executing JS callback");
                }
            } else {
                qCritical("Provided JS object is not callable");
            }
        }
    }

    void connectProvider(InterfaceBase &provider);

    InterfaceBase *providerPrivate()
    {
        return m_provider;
    }

    friend class IPCAttachedPropertyFactoryBase;
    friend class IPCServiceAdapterBase;
    friend class NewIPCServiceAdapterBase;

private:
    InterfaceBase *m_provider = nullptr;
    QQmlEngine* m_qmlEngine = nullptr;

};

/*!
 * This is the class which is registered when calling registerQmlComponent()
 * It is an actual instance of the QMLFrontendType and wraps an instance of the provider
 */
template<typename ProviderImplementationType>
class QMLFrontendByReference : public ProviderImplementationType::QMLFrontendType
{

public:
    QMLFrontendByReference(ProviderImplementationType& provider, QObject *parent = nullptr) : ProviderImplementationType::QMLFrontendType(parent), m_provider(provider)
    {
    }

    QMLFrontendByReference(ProviderImplementationType& provider, QQmlEngine *engine) : ProviderImplementationType::QMLFrontendType(engine), m_provider(provider)
    {
    }

    const QString &implementationID() override
    {
        return m_provider.implementationID();
    }

    void componentComplete() override
    {
        // notify anyone interested that we are ready (such as an IPC attached property)
        m_provider.setComponentCompleted();
    }

    void connectProvider() {
        ProviderImplementationType::QMLFrontendType::connectProvider(m_provider);
    }

private:
    ProviderImplementationType& m_provider;

};

template<typename ProviderImplementationType>
class TQMLFrontend : public QMLFrontendByReference<ProviderImplementationType> {

public:

    TQMLFrontend(QObject *parent = nullptr) : QMLFrontendByReference<ProviderImplementationType>(m_provider, parent)
    {
        this->connectProvider();
    }

    TQMLFrontend(QQmlEngine *engine) : QMLFrontendByReference<ProviderImplementationType>(m_provider, engine)
    {
        this->connectProvider();
    }

private:
    ProviderImplementationType m_provider;

};


template<typename Type>
QObject *singletonGetter(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(scriptEngine);
    Q_UNUSED(engine);
    auto obj = new Type(engine);
    obj->componentComplete();
    qDebug() << "Singleton created" << obj;
    return obj;
}


template<typename ProviderType, ProviderType& (*getter)()>
QObject *singletonGetterByFunction(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(scriptEngine);
    Q_UNUSED(engine);
    auto obj = new QMLFrontendByReference<ProviderType>(getter());
    obj->connectProvider();
    obj->componentComplete();
    qDebug() << "Singleton created" << obj;
    return obj;
}


/*!
 * Register the given interface QML implementation as a creatable QML component.
 * By default, the component is registered under the same name as defined in the QFace definition.
 */
template<typename ProviderType>
int registerQmlComponent(const char *uri, const char *name = ProviderType::INTERFACE_NAME, int majorVersion =
        ProviderType::VERSION_MAJOR,
        int minorVersion = ProviderType::VERSION_MINOR,
        typename std::enable_if<std::is_base_of<facelift::InterfaceBase, ProviderType>::value>::type * = nullptr)
{
    ProviderType::registerTypes(uri);
    return ::qmlRegisterType<TQMLFrontend<ProviderType> >(uri, majorVersion, minorVersion, name);
}


/*!
 * Register the given implementation type as a singleton QML component.
 * By default, the component is registered under the same name as defined in the QFace definition.
 */
template<typename ProviderType>
int registerSingletonQmlComponent(const char *uri, const char *name = ProviderType::INTERFACE_NAME,
        int majorVersion = ProviderType::VERSION_MAJOR,
        int minorVersion = ProviderType::VERSION_MINOR,
        typename std::enable_if<std::is_base_of<facelift::InterfaceBase, ProviderType>::value>::type * = nullptr)
{
    ProviderType::registerTypes(uri);
    typedef TQMLFrontend<ProviderType> QMLType;
    return ::qmlRegisterSingletonType<QMLType>(uri, majorVersion, minorVersion, name, &singletonGetter<QMLType>);
}


/*!
 * Register the given implementation type as a singleton QML component, with the given getter function.
 * By default, the component is registered under the same name as defined in the QFace definition.
 */
template<typename ProviderType, ProviderType& (*singletonGetterFunction)()>
int registerSingletonQmlComponent(const char *uri,
        const char *name = ProviderType::INTERFACE_NAME,
        int majorVersion = ProviderType::VERSION_MAJOR,
        int minorVersion = ProviderType::VERSION_MINOR,
        typename std::enable_if<std::is_base_of<facelift::InterfaceBase, ProviderType>::value>::type * = nullptr)
{
    ProviderType::registerTypes(uri);
    typedef QMLFrontendByReference<ProviderType> QMLType;
    return ::qmlRegisterSingletonType<QMLType>(uri, majorVersion, minorVersion, name, &singletonGetterByFunction<ProviderType, singletonGetterFunction>);
}


template<typename ProviderType>
typename ProviderType::QMLFrontendType *getQMLFrontend(ProviderType *provider)
{
    if (provider == nullptr) {
        return nullptr;
    } else {
        if (provider->m_qmlFrontend == nullptr) {
            // No QML frontend instantiated yet => create one
            provider->m_qmlFrontend = new typename ProviderType::QMLFrontendType(provider);
            provider->m_qmlFrontend->connectProvider(*provider);
        }
        return provider->m_qmlFrontend;
    }
}


class FaceliftModelLib_EXPORT ModelListModelBase : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    typedef size_t (QObject::*SizeGetterFunction)();

    Q_SIGNAL void countChanged();

    ModelListModelBase();

    /**
     * Return the element at the given row index
     */
    Q_INVOKABLE QVariant get(int rowIndex) const;

    void setModelProperty(facelift::ModelBase &property);

    int rowCount(const QModelIndex &index = QModelIndex()) const override;

    QHash<int, QByteArray> roleNames() const override;

private:

    void onBeginResetModel();

    void onEndResetModel();

    void onBeginInsertElements(int first, int last);

    void onEndInsertElements();

    void onBeginRemoveElements(int first, int last);

    void onEndRemoveElements();

    void onDataChanged(int first, int last);

protected:
    facelift::ModelBase *m_property = nullptr;

};

template<typename ElementType>
class ModelListModel : public ModelListModelBase
{
public:
    typedef ElementType (QObject::*ElementGetterFunction)(size_t);

    ModelListModel()
    {
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        Q_UNUSED(role);
        auto element = m_property->elementAt(index.row());
        return QVariant::fromValue(element);
    }

    void setModelProperty(facelift::Model<ElementType> &property)
    {
        if (m_property != &property) {
            m_property = &property;
            ModelListModelBase::setModelProperty(property);
        }
    }

private:
    facelift::Model<ElementType> *m_property = nullptr;

};

}
