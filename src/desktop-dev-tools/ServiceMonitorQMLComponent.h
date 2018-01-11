/*
 *   This is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 */

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
                    qWarning() << "Creating monitor for " << targetInterface;
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
