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
#include <functional>

#include <QObject>
#include <QDebug>
#include <QMap>
#include <qqml.h>

#include "FaceliftCommon.h"
#include "AsyncAnswer.h"

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
    typedef void QMLAdapterType;

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

    PropertyInterface(Class *object, GetterMethod getter, ChangeSignal signal, const char* name)
    {
        m_object = object;
        m_signal = signal;
        m_getter = getter;
        m_name = name;
    }

    const PropertyType &value() const
    {
        const auto &v = (m_object->*m_getter)();
        return v;
    }

    ChangeSignal signal() const
    {
        return m_signal;
    }

    GetterMethod getter() const
    {
        return m_getter;
    }

    Class *object() const
    {
        return m_object;
    }

    const char *name() const
    {
        return m_name;
    }

private:
    Class *m_object = nullptr;
    ChangeSignal m_signal;
    GetterMethod m_getter;
    const char* m_name = nullptr;
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
    ModelPropertyInterface(Class *object, facelift::Model<PropertyType> &property)
    {
        m_object = object;
        m_property = &property;
    }

    facelift::Model<PropertyType> *property() const
    {
        return m_property;
    }

    Class *object() const
    {
        return m_object;
    }

private:
    Class *m_object = nullptr;
    facelift::Model<PropertyType> *m_property = nullptr;
};


template<typename Class, typename ServiceType>
class ServicePropertyInterface
{
public:
    typedef void (Class::*ChangeSignal)();
    typedef ServiceType * (Class::*GetterMethod)();

    ServicePropertyInterface(Class *object, GetterMethod getter, ChangeSignal signal)
    {
        m_object = object;
        m_signal = signal;
        m_getter = getter;
    }

    ServiceType *value() const
    {
        return (m_object->*m_getter)();
    }

    ChangeSignal signal() const
    {
        return m_signal;
    }

    Class *object() const
    {
        return m_object;
    }

    GetterMethod getter() const
    {
        return m_getter;
    }

private:
    Class *m_object = nullptr;
    ChangeSignal m_signal;
    GetterMethod m_getter;
};


FaceliftModelLib_EXPORT void registerInterfaceImplementationInstance(InterfaceBase & i);

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
