#pragma once

#include "Model.h"

namespace facelift {


class ServiceWrapperBase
{

protected:
    void addConnection(QMetaObject::Connection connection)
    {
        m_connections.append(connection);
    }

    void reset()
    {
        for (const auto &connection : m_connections) {
            auto successfull = QObject::disconnect(connection);
            Q_ASSERT(successfull);
        }
    }

    QVector<QMetaObject::Connection> m_connections;  /// The list of connections which this property is bound to

};


/**
 *
 */
template<typename WrappedType>
class ServiceWrapper : public WrappedType, public ServiceWrapperBase
{

public:
    void setWrapped(WrappedType *wrapped)
    {
        m_wrapped = wrapped;

        if (!m_wrapped.isNull()) {
            reset();
        }

        initConnections();
    }

    virtual ~ServiceWrapper()
    {
    }

protected:
    ServiceWrapper(QObject *parent) : WrappedType(parent)
    {
    }

    WrappedType *wrapped() const
    {
        Q_ASSERT(!m_wrapped.isNull());
        return m_wrapped.data();
    }

    virtual void initConnections(WrappedType *wrapped) = 0;

private:
    QPointer<WrappedType> m_wrapped;

};

}
