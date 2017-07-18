#pragma once

#include "mypackage/MyInterfacePropertyAdapter.h"


using namespace mypackage;

/**
 * C++ Implementation of the MyInterface API
 */
class MyInterfaceCppImplementation :
    public MyInterfacePropertyAdapter
{

public:
    MyInterfaceCppImplementation(QObject *parent = nullptr) :
        MyInterfacePropertyAdapter(parent)
    {
        connect(&m_timer, &QTimer::timeout, this, [this] () {
            m_counter++;   // The value change signal is automatically triggered for you here
        });
        m_timer.start(1000);
    }

    void resetCounter() override
    {
        m_counter = 0;
        counterReset();
    }

private:
    QTimer m_timer;

};
