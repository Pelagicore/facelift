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

#include "ModelQMLImplementationBase.h"

namespace facelift {


ModelQMLImplementationBase::ModelQMLImplementationBase(QObject *parent) :
    QObject(parent)
{
}


bool ModelQMLImplementationBase::ready() const
{
    Q_ASSERT(m_interface != nullptr);
    return m_interface->ready();
}

void ModelQMLImplementationBase::setImplementationID(const QString &id)
{
    m_implementationID = id;
    assignImplementationID();
}

const QString &ModelQMLImplementationBase::implementationID() const
{
    return m_implementationID;
    Q_ASSERT(m_interface != nullptr);
    return m_interface->implementationID();
}

void ModelQMLImplementationBase::setInterface(InterfaceBase *i)
{
    m_interface = i;
    QObject::connect(i, &InterfaceBase::readyChanged, this, &ModelQMLImplementationBase::readyChanged);

    assignImplementationID();
    QObject::connect(this, &QObject::objectNameChanged, this, &ModelQMLImplementationBase::assignImplementationID);
}

QJSValue &ModelQMLImplementationBase::checkMethod(QJSValue &method, const char *methodName)
{
    if (!method.isCallable()) {
        qFatal("Method \"%s\" of Facelift interface implementation \"%s\" is about to be called but it is not implemented in your QML file. "
                "That method MUST be implemented if it is called.", qPrintable(methodName), qPrintable(interfac()->interfaceID()));
    }

    return method;
}

QQmlListProperty<QObject> ModelQMLImplementationBase::childItems()
{
    return QQmlListProperty<QObject>(this, m_children);
}

void ModelQMLImplementationBase::classBegin()
{
}

void ModelQMLImplementationBase::componentComplete()
{
    assignImplementationID();
    m_interface->setComponentCompleted();
}

void ModelQMLImplementationBase::assignImplementationID()
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

QQmlEngine* ModelQMLImplementationBase::qmlEngine()
{
    return ::qmlEngine(this);
}


void ModelQMLImplementationBase::onInvalidQMLEngine() const
{
    faceliftSeriousError("QML engine is not available");
}


}
