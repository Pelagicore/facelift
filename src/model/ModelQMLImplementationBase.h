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


#include "QMLAdapter.h"

namespace facelift {

/**
 * Base class for QML-based interface implementations
 */
class ModelQMLImplementationBase : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

public:
    // We set a default property so that we can have children QML elements in our QML implementations, such as Timer
    Q_PROPERTY(QQmlListProperty<QObject> childItems READ childItems)
    Q_CLASSINFO("DefaultProperty", "childItems")

    ModelQMLImplementationBase(QObject *parent = nullptr);

    Q_PROPERTY(bool ready READ ready WRITE setReady NOTIFY readyChanged)

    virtual void setReady(bool ready) = 0;

    bool ready() const;

    Q_SIGNAL void readyChanged();

    Q_PROPERTY(QString implementationID READ implementationID WRITE setImplementationID NOTIFY implementationIDChanged)

    void setImplementationID(const QString &id);

    const QString &implementationID() const;

    Q_SIGNAL void implementationIDChanged();

    void setInterface(InterfaceBase *i);

    QJSValue &checkMethod(QJSValue &method, const char *methodName);

    QQmlListProperty<QObject> childItems();

    void classBegin() override;

    void componentComplete() override;

    facelift::InterfaceBase *interfac() const
    {
        return m_interface;
    }

    void assignImplementationID();

    QQmlEngine* qmlEngine();

    void onInvalidQMLEngine() const;

private:
    QList<QObject *> m_children;
    InterfaceBase *m_interface = nullptr;
    QString m_implementationID;
};


}
