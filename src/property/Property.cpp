#include "Property.h"

namespace facelift {

PropertyBase::PropertyBase()
{
}

PropertyBase::~PropertyBase()
{
}

void PropertyBase::doBreakBinding() {
    qDebug() << this->name() << " property : breaking binding";

    for (const auto &connection : m_connections) {
        auto successfull = QObject::disconnect(connection);
        Q_ASSERT(successfull);
    }
    m_connections.clear();
}


void PropertyBase::triggerValueChangedSignal() {
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
    if (signalPointer() != nullptr) {
        if (isDirty()) {
            qDebug() << "Property" << name() << ": Triggering notification. New value:" << toString();
            // Trigger the signal
            clean();
            (m_ownerObject->*signalPointer())();
        }
    }
}

}
