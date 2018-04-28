#include "FaceliftProperty.h"

namespace facelift {

PropertyBase::PropertyBase()
{
}

PropertyBase::~PropertyBase()
{
}

void PropertyBase::doBreakBinding()
{
    qDebug() << this->name() << " property : breaking binding";

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
    if (signalPointer() != nullptr) {
        if (isDirty()) {
            qDebug() << "Property" << name() << ": Triggering notification. New value:" << toString();
            // Trigger the signal
            clean();
            (m_ownerObject->*signalPointer())();
        }
    }
}

void PropertyBase::setGetterFunctionContext(QObject *context)
{
    if (m_getterFunctionContext != nullptr) {
        QObject::disconnect(m_getterFunctionContextConnection);
    }
    m_getterFunctionContext = context;

    if (m_getterFunctionContext != nullptr) {
        m_getterFunctionContextConnection = QObject::connect(m_getterFunctionContext, &QObject::destroyed, owner(), [this]() {
                m_getterFunctionContext = nullptr;
            });
    }
}

}
