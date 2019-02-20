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

#include <memory>
#include <QObject>
#include <QDebug>
#include <QMap>
#include <qqml.h>
#include <QPointer>

#include "FaceliftCommon.h"

#if defined(FaceliftModelLib_LIBRARY)
#  define FaceliftModelLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftModelLib_EXPORT Q_DECL_IMPORT
#endif


namespace facelift {

template<typename ElementType>
using Map = QMap<QString, ElementType>;


template<typename InterfaceType, typename PropertyType>
using PropertyGetter = const PropertyType &(*)();

/**
 * Base interface which every interface inherits from
 */
class FaceliftModelLib_EXPORT InterfaceBase : public QObject
{
    Q_OBJECT

public:
    typedef void QMLFrontendType;

    InterfaceBase(QObject *parent = nullptr) :
        QObject(parent)
    {
    }

    void setImplementationID(const QString &id)
    {
        m_implementationID = id;
    }

    const QString &implementationID() const
    {
        return m_implementationID;
    }

    virtual bool ready() const = 0;

    Q_SIGNAL void readyChanged();

    QObject *impl()
    {
        return this;
    }

    void init(const QString &interfaceName);

    const QString &interfaceID() const
    {
        return m_interfaceName;
    }

    void setComponentCompleted() {
        if (!m_componentCompleted) {
            m_componentCompleted = true;
            emit componentCompleted();
        }
    }

    bool isComponentCompleted() const {
        return m_componentCompleted;
    }

    Q_SIGNAL void componentCompleted();

protected:
    friend class ModelQMLImplementationBase;

private:
    QString m_implementationID = "Undefined";
    QString m_interfaceName;

    bool m_componentCompleted = false;

};

template<typename QMLType>
void qmlRegisterType(const char *uri, const char *typeName)
{
    ::qmlRegisterType<QMLType>(uri, 1, 0, typeName);
}

template<typename QMLType>
void qmlRegisterType(const char *uri)
{
    ::qmlRegisterType<QMLType>(uri, QMLType::INTERFACE_NAME);
}


template<typename Class, typename PropertyType>
class PropertyInterface
{

public:
    typedef void (Class::*ChangeSignal)();
    typedef const PropertyType &(Class::*GetterMethod)() const;

    PropertyInterface(Class *o, GetterMethod g, ChangeSignal s)
    {
        object = o;
        signal = s;
        getter = g;
    }

    const PropertyType &value() const
    {
        const auto &v = (object->*getter)();
        return v;
    }

    Class *object;
    ChangeSignal signal;
    GetterMethod getter;

};


class FaceliftModelLib_EXPORT ModelBase : public QObject
{
    Q_OBJECT

public:

    ModelBase();

    /**
     * See https://doc.qt.io/qt-5/qabstractitemmodel.html#dataChanged
     */
    Q_SIGNAL void dataChanged(int first, int last);

    /**
     * See https://doc.qt.io/qt-5/qabstractitemmodel.html#dataChanged
     */
    void dataChanged(int index)
    {
        dataChanged(index, index);
    }

    /**
     * See https://doc.qt.io/qt-5/qabstractitemmodel.html#beginInsertRows
     */
    Q_SIGNAL void beginInsertElements(int first, int last);

    /**
     * See https://doc.qt.io/qt-5/qabstractitemmodel.html#beginRemoveRows
     */
    Q_SIGNAL void beginRemoveElements(int first, int last);

    /**
     * See https://doc.qt.io/qt-5/qabstractitemmodel.html#beginMoveRows
     */
    Q_SIGNAL void beginMoveElements(int sourceFirst, int sourceLast, int destinationRow);

    /**
     * See https://doc.qt.io/qt-5/qabstractitemmodel.html#endMoveRows
     */
    Q_SIGNAL void endMoveElements();

    /**
     * See https://doc.qt.io/qt-5/qabstractitemmodel.html#endInsertRows
     */
    Q_SIGNAL void endInsertElements();

    /**
     * See https://doc.qt.io/qt-5/qabstractitemmodel.html#endRemoveRows
     */
    Q_SIGNAL void endRemoveElements();

    /**
     * See https://doc.qt.io/qt-5/qabstractitemmodel.html#beginResetModel
     */
    Q_SIGNAL void beginResetModel();

    /**
     * See https://doc.qt.io/qt-5/qabstractitemmodel.html#endResetModel
     */
    Q_SIGNAL void endResetModel();

    Q_SIGNAL void elementCountChanged();

    int size() const
    {
        return m_size;
    }

    void bindOtherModel(facelift::ModelBase *otherModel);

protected:
    void setSize(int size)
    {
        m_size = size;
    }

private:
    void onModelChanged();
    void applyNewSize();

    int m_size = 0;
    int m_previousElementCount = 0;
    int m_pendingSize = -1;

protected:
    bool m_resettingModel = false;

};


template<typename ElementType>
class Model : public ModelBase
{
public:
    virtual ElementType elementAt(int index) const = 0;

};


template<typename Class, typename PropertyType>
class ModelPropertyInterface
{
public:
    ModelPropertyInterface(Class* o, facelift::Model<PropertyType>& p)
    {
        object = o;
        property = &p;
    }

    Class* object;
    facelift::Model<PropertyType>* property = nullptr;
};


template<typename Class, typename ServiceType>
class ServicePropertyInterface
{
public:
    typedef void (Class::*ChangeSignal)();
    typedef ServiceType * (Class::*GetterMethod)();

    ServicePropertyInterface(Class *o, GetterMethod g, ChangeSignal s)
    {
        object = o;
        signal = s;
        getter = g;
    }

    ServiceType *value() const
    {
        return (object->*getter)();
    }

    Class *object;
    ChangeSignal signal;
    GetterMethod getter;

};





template<typename CallBack>
class TAsyncAnswerMaster
{

public:
    TAsyncAnswerMaster(QObject* context, CallBack callback) : m_callback(callback)
    {
        m_context = context;
    }

    ~TAsyncAnswerMaster()
    {
        if (!m_isAlreadyAnswered) {
            qWarning() << "No answer provided to asynchronous call";
        }
    }

    template<typename ... Types>
    void call(const Types & ... args)
    {
        setAnswered();
        if (m_context)
            m_callback(args ...);
    }

private:
    void setAnswered()
    {
        Q_ASSERT(m_isAlreadyAnswered == false);
        m_isAlreadyAnswered = true;
    }

protected:
    CallBack m_callback;
    bool m_isAlreadyAnswered = false;
    QPointer<QObject> m_context;
};


template<typename ReturnType>
class AsyncAnswer
{
    typedef std::function<void (const ReturnType &)> CallBack;

public:
    class Master : public TAsyncAnswerMaster<CallBack>
    {
    public:
        using TAsyncAnswerMaster<CallBack>::m_callback;
        Master(QObject* context, CallBack callback) : TAsyncAnswerMaster<CallBack>(context, callback)
        {
        }
    };

    AsyncAnswer()
    {
    }

    AsyncAnswer(QObject* context, CallBack callback) : m_master(new Master(context, callback))
    {
    }

    AsyncAnswer(const AsyncAnswer &other) : m_master(other.m_master)
    {
    }

    AsyncAnswer &operator=(const AsyncAnswer &other)
    {
        m_master = other.m_master;
        return *this;
    }

    void operator()(const ReturnType &returnValue) const
    {
        if (m_master) {
            m_master->call(returnValue);
        }
    }

private:
    std::shared_ptr<Master> m_master;
};

template<>
class AsyncAnswer<void>
{
    typedef std::function<void ()> CallBack;

public:
    class Master : public TAsyncAnswerMaster<CallBack>
    {
    public:
        using TAsyncAnswerMaster<CallBack>::m_callback;

        Master(QObject* context, CallBack callback) : TAsyncAnswerMaster<CallBack>(context, callback)
        {
        }
    };

    AsyncAnswer()
    {
    }

    AsyncAnswer(QObject* context, CallBack callback) : m_master(new Master(context, callback))
    {
    }

    AsyncAnswer(const AsyncAnswer &other) : m_master(other.m_master)
    {
    }

    void operator()() const
    {
        if (m_master) {
            m_master->call();
        }
    }

private:
    std::shared_ptr<Master> m_master;
};

}

template<typename ElementType>
inline QTextStream &operator<<(QTextStream &outStream, const facelift::Map<ElementType> &f)
{
    outStream << "[";
    for (const auto &e : f.toStdMap()) {
        outStream << e.first << "=" << e.second << ", ";
    }
    outStream << "]";
    return outStream;
}


Q_DECLARE_METATYPE(facelift::InterfaceBase *)
