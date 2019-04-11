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

#include "FaceliftProperty.h"
#include <QTimer>

namespace facelift {

PropertyBase::PropertyBase()
{
}

PropertyBase::~PropertyBase()
{
}

void PropertyBase::doBreakBinding()
{
    qCDebug(LogModel) << this->name() << " property : breaking binding";

    for (const auto &connection : m_connections) {
        QObject::disconnect(connection);
    }
    m_connections.clear();
}


void PropertyBase::triggerValueChangedSignal()
{
    if (m_asynchronousNotification) {
        // Asynchronous notification is enabled => we will actually trigger the change signal during the next main loop iteration
        if (!m_notificationTimerEnabled) {
            m_notificationTimerEnabled = true;

            QTimer::singleShot(0, m_ownerObject, [this] () {
                    doTriggerChangeSignal();
                    m_notificationTimerEnabled = false;
                });

        }
    } else {
        doTriggerChangeSignal();
    }
}

void PropertyBase::doTriggerChangeSignal()
{
    setReady(true);

    if (signalPointer() != nullptr) {
        if (isDirty()) {
            qCDebug(LogModel) << "Property" << name() << ": Triggering notification. New value:" << toString();
            // Trigger the signal
            clean();
            (m_ownerObject->*signalPointer())();
        }
    }
}

}
