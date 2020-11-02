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

#include "QMLAdapterBase.h"

namespace facelift {

const QJSValue QMLAdapterBase::NO_OPERATION_JS_CALLBACK = QJSValue(QJSValue::SpecialValue::UndefinedValue);

QMLAdapterBase::QMLAdapterBase(QObject *parent) : QObject(parent)
{
}

/**
 *  This constructor is used when instantiating a singleton
 */
QMLAdapterBase::QMLAdapterBase(QQmlEngine *engine) : QMLAdapterBase(static_cast<QObject*>(engine))
{
    // store the reference to the engine since we can not get it from the "qmlEngine()" global function
    m_qmlEngine = engine;
}

InterfaceBase *QMLAdapterBase::provider() {
    Q_ASSERT(m_provider != nullptr);
    qCWarning(LogModel) << "Accessing private provider implementation object";
    return m_provider;
}

bool QMLAdapterBase::ready() const
{
    return m_provider->ready();
}

const QString &QMLAdapterBase::implementationID()
{
   static QString id;
   return id;
}

void QMLAdapterBase::classBegin()
{
}

void QMLAdapterBase::setParentQMLAdapter(QMLAdapterBase * parentQMLAdapter)
{
    // One would expect that this method is not called multiple times with different parentQMLAdapter,
    // but it actually is, in in-process IPC scenarios. TODO: clarify and fix
    if (m_parentQMLAdapter == nullptr) {
        m_parentQMLAdapter = parentQMLAdapter;
        Q_ASSERT(m_parentQMLAdapter);
        m_qmlEngine = m_parentQMLAdapter->qmlEngine();
    }
}


QQmlEngine* QMLAdapterBase::qmlEngine() const
{
    if (m_qmlEngine == nullptr) {
        m_qmlEngine = ::qmlEngine(this);
        Q_ASSERT(m_qmlEngine);
    }
    return m_qmlEngine;
}


void QMLAdapterBase::connectProvider(InterfaceBase &provider)
{
    m_provider = &provider;
    connect(m_provider, &InterfaceBase::readyChanged, this, &QMLAdapterBase::readyChanged);
}


}
