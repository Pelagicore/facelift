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

#include <QWidget>

#include "ServiceMonitor.h"
#include "QMLModel.h"

namespace facelift {

class ServiceMonitorQMLComponent : public QObject
{
    Q_OBJECT

public:
    ServiceMonitorQMLComponent(QObject *parent = nullptr) :
        QObject(parent)
    {
        m_window.setLayout(&m_layout);
        m_window.resize(600, 800);
        m_window.show();
    }

    Q_PROPERTY(QObject * target READ target WRITE setTarget NOTIFY targetChanged)

    void setTarget(QObject *target)
    {
        if (m_target != target) {
            delete m_monitor;
            m_monitor = nullptr;
            m_target = target;
            if (m_target != nullptr) {
                auto targetInterface = qobject_cast<InterfaceBase *>(m_target);
                if (targetInterface == nullptr) {
                    auto qmlFrontend = qobject_cast<QMLFrontendBase *>(m_target);
                    if (qmlFrontend != nullptr) {
                        targetInterface = qmlFrontend->provider();
                    } else {
                        auto qmlImpl = qobject_cast<ModelQMLImplementationBase *>(m_target);
                        if (qmlImpl != nullptr) {
                            targetInterface = qmlImpl->interfac();
                        }
                    }
                }

                if (targetInterface != nullptr) {
                    qCWarning(LogGeneral) << "Creating monitor for " << targetInterface;
                    m_monitor = ServiceMonitorManager::instance().createMonitor(targetInterface);
                    m_window.setWindowTitle(m_monitor->mainWidget()->windowTitle());
                    m_layout.addWidget(m_monitor->mainWidget());
                } else {
                    qFatal("Bad object type %s", qPrintable(m_target->metaObject()->className()));
                }
            }
            targetChanged();
        }
    }

    Q_SIGNAL void targetChanged();

    QObject *target() const
    {
        return m_target;
    }

private:
    QObject *m_target;
    ServiceMonitorBase *m_monitor = nullptr;
    QWidget m_window;
    QHBoxLayout m_layout;
};

}
