/**********************************************************************
**
** Copyright (C) 2020 Luxoft Sweden AB
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
#include <QQmlEngine>
#include <QtQml>
#include <QQmlParserStatus>

#include "FaceliftModel.h"

namespace facelift {

/*!
 * This is the base class which all QML frontends extend
 */
class QMLAdapterBase : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

public:

    static const QJSValue NO_OPERATION_JS_CALLBACK;

    QMLAdapterBase(QObject *parent);

    /**
     *  This constructor is used when instantiating a singleton
     */
    QMLAdapterBase(QQmlEngine *engine);

    Q_PROPERTY(QObject * provider READ provider CONSTANT)
    virtual InterfaceBase *provider();

    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    bool ready() const;

    Q_SIGNAL void readyChanged();

    Q_PROPERTY(QString implementationID READ implementationID CONSTANT)
    virtual const QString &implementationID();

    void classBegin() override;

    void componentComplete() override
    {
       m_provider->setComponentCompleted();
    }

    QQmlEngine* qmlEngine() const;

    void setParentQMLAdapter(QMLAdapterBase * parentQMLAdapter);

protected:

    void connectProvider(InterfaceBase &provider);

    InterfaceBase *providerPrivate()
    {
        return m_provider;
    }

    friend class IPCAttachedPropertyFactory;
    friend class IPCServiceAdapterBase;
    friend class NewIPCServiceAdapterBase;

private:
    InterfaceBase *m_provider = nullptr;
    mutable QQmlEngine* m_qmlEngine = nullptr;
    QMLAdapterBase* m_parentQMLAdapter = nullptr;
};


}
