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

#include <gtest/gtest.h>
#include "IPCProxyBase.h"
#include "InterfaceBase.h"
#include "observer.h"
#include <QSignalSpy>

namespace {

using namespace facelift;

// MOCK for function
int foo(int val){
    std::cout << " the input variable is: " << val << std::endl;
    return val;
}

// MOCK class for callback
class Counter
{
public:
    Counter() : m_value{0} {}

    void incValue() {
        ++m_value;
    }
    void addValue(int val) {
        m_value += val;
    }
    int getValue() const {
       return m_value;
    }

private:
    int m_value;
};

// MOCK class for IPCProxy
template<typename AdapterType>
class IPCProxy : public IPCProxyBase<AdapterType>
{
    bool m_serviceReady{};

public:
    IPCProxy(QObject *parent): IPCProxyBase<AdapterType>(parent) {}

    bool readyTest() const {
        return m_serviceReady;
    }

    bool setReadyTest(bool ready) {
        return m_serviceReady = ready;
    }
};

class IPCProxyTest : public ::testing::Test
{
public:
    // Objects containing executable functions
    Counter c1;
    Counter c2;
    Counter c3;
    // The "Proxy" is the source of the signal of interest
    IPCProxy<InterfaceBase> proxy { nullptr };
    // The "IsReadyObserver" is the watcher of the signal of interest in "Proxy" and contain observers to execute: "callOnReady", "callOnceReady"
    IsReadyObserver readyObserver{};

    ~IPCProxyTest() {}
};
TEST_F(IPCProxyTest, addDeleteObservers)
{
    // Checking correctness of add and delete observers
    // Add to readyObserver a 2 observers, then delete 1 observer, then add 1 observer, then delete each of them

    // Set state ready() of Proxy to disable
    proxy.setReadyTest(false);

    // Setting up an "IsReadyObserver" to watch out for signal in "Proxy"
    readyObserver.watch(&proxy, &IPCProxy<InterfaceBase>::readyChanged, &IPCProxy<InterfaceBase>::readyTest);

    // Set observers to "IsReadyObserver"
    const auto sso = readyObserver.setStandartObserver(&c1, &Counter::incValue);
    const auto sto1 = readyObserver.setSingleTimeObserver(&c2, &Counter::incValue);
    ASSERT_EQ(readyObserver.count(), 2);
    readyObserver.delObserver(sso);
    ASSERT_EQ(readyObserver.count(), 1);
    const auto sto2 = readyObserver.setSingleTimeObserver(&c3, &Counter::incValue);
    ASSERT_EQ(readyObserver.count(), 2);
    readyObserver.delObserver(sto2);
    ASSERT_EQ(readyObserver.count(), 1);
    readyObserver.delObserver(sto1);
    ASSERT_EQ(readyObserver.count(), 0);
}
TEST_F(IPCProxyTest, clearObservers)
{
    // Checking correctness of clearing observers
    // Add to readyObserver a some observers, then clear them

    // Set state ready() of Proxy to disable
    proxy.setReadyTest(false);

    // Setting up an "IsReadyObserver" to watch out for signal in "Proxy"
    readyObserver.watch(&proxy, &IPCProxy<InterfaceBase>::readyChanged, &IPCProxy<InterfaceBase>::readyTest);

    // Set observers to "IsReadyObserver"
    readyObserver.setStandartObserver(&c1, &Counter::incValue);
    readyObserver.setSingleTimeObserver(&c2, &Counter::incValue);
    readyObserver.setSingleTimeObserver(&c3, &Counter::incValue);
    ASSERT_EQ(readyObserver.count(), 3);
    readyObserver.clear();
    ASSERT_EQ(readyObserver.count(), 0);
}
TEST_F(IPCProxyTest, proxyReadyIsFalse)
{
     // Checking if proxy is not ready and signal exists, observer should not be executed.
     // Set to Proxy in state "false" and set 2 observers

    // Set state ready() of Proxy to disable
    proxy.setReadyTest(false);

    // Setting up an "IsReadyObserver" to watch out for signal in "Proxy"
    readyObserver.watch(&proxy, &IPCProxy<InterfaceBase>::readyChanged, &IPCProxy<InterfaceBase>::readyTest);
    // Set observers to "IsReadyObserver"
    readyObserver.setStandartObserver(&c1, &Counter::incValue);
    readyObserver.setSingleTimeObserver(&c2, &Counter::incValue);

    // Check values before calling a signals
    ASSERT_EQ(c1.getValue(), 0); // for StandartObserver ("callOnReady")
    ASSERT_EQ(c2.getValue(), 0); // for SingleTimeObserver ("callOnceReady")

    // Set signal tracking for "IsReadyObserver"
    QSignalSpy spyObserver(&readyObserver, &IsReadyObserver::readyChanged);
    ASSERT_EQ(spyObserver.isValid(), true);
    spyObserver.clear();

    // Set signal tracking for "Proxy"
    QSignalSpy spyProxy(&proxy, &IPCProxy<InterfaceBase>::readyChanged );
    ASSERT_EQ(spyProxy.isValid(), true);
    spyProxy.clear();

    // Check that the signals on "IsReadyObserver" and "Proxy" are absent
    ASSERT_EQ(spyObserver.count(), 0);
    ASSERT_EQ(spyProxy.count(), 0);

    // Generate a some signals on the "Proxy" which is tracking on "IsReadyObserver"
    for (auto i=0; i<3; ++i) {
        proxy.readyChanged();
    }

    // Check the number of received signals on "Proxy"
    ASSERT_EQ(spyProxy.count(), 3);
    // Check the number of received signals on "IsReadyObserver"
    ASSERT_EQ(spyObserver.count(), 0);

    // Check values after signals call
    ASSERT_EQ(c1.getValue(), 0); // for StandartObserver ("callOnReady")
    ASSERT_EQ(c2.getValue(), 0); // for SingleTimeObserver ("callOnceReady")
}
TEST_F(IPCProxyTest, proxyReadyIsTrue)
{
    // Checking if proxy is ready and signal exists, all observer should be executed.
    // Set to Proxy in state "true" and set 2 observers

    // Set state ready() of Proxy to enable
    proxy.setReadyTest(true);

    // Setting up an "IsReadyObserver" to watch out for signal in "Proxy"
    readyObserver.watch(&proxy, &IPCProxy<InterfaceBase>::readyChanged, &IPCProxy<InterfaceBase>::readyTest);
    // Set observers to "IsReadyObserver"
    readyObserver.setStandartObserver(&c1, &Counter::incValue);
    readyObserver.setSingleTimeObserver(&c2, &Counter::incValue);
    readyObserver.setStandartObserver( []() { foo(42); } );

    // Check values before calling a signals
    ASSERT_EQ(c1.getValue(), 0); // for StandartObserver ("callOnReady")
    ASSERT_EQ(c2.getValue(), 0); // for SingleTimeObserver ("callOnceReady")

    // Set signal tracking for "IsReadyObserver"
    QSignalSpy spyObserver(&readyObserver, &IsReadyObserver::readyChanged);
    ASSERT_EQ(spyObserver.isValid(), true);
    spyObserver.clear();

    // Set signal tracking for "Proxy"
    QSignalSpy spyProxy(&proxy, &IPCProxy<InterfaceBase>::readyChanged );
    ASSERT_EQ(spyProxy.isValid(), true);
    spyProxy.clear();

    // Check that the signals on "IsReadyObserver" and "Proxy" are absent
    ASSERT_EQ(spyObserver.count(), 0);
    ASSERT_EQ(spyProxy.count(), 0);

    // Generate a some signals on the "Proxy" which is tracking on "IsReadyObserver"
    for (auto i=0; i<3; ++i) {
        proxy.readyChanged();
    }

    // Check the number of received signals on "Proxy"
    ASSERT_EQ(spyProxy.count(), 3);
    // Check the number of received signals on "IsReadyObserver"
    ASSERT_EQ(spyObserver.count(), 3);

    // Check values after signals call
    ASSERT_EQ(c1.getValue(), 3); // for StandartObserver ("callOnReady")
    ASSERT_EQ(c2.getValue(), 1); // for SingleTimeObserver ("callOnceReady")
}
TEST_F(IPCProxyTest, setNullptrObservers)
{
     // Checking if proxy is ready and signal exists and set nullptr instead address of observer, all valid observer should be executed.
     // Set to Proxy in state "true" and set 2 observers and 1 nullptr

    // Set state ready() of Proxy to enable
    proxy.setReadyTest(true);

    // Setting up an "IsReadyObserver" to watch out for signal in "Proxy"
    readyObserver.watch(&proxy, &IPCProxy<InterfaceBase>::readyChanged, &IPCProxy<InterfaceBase>::readyTest);
    // Set observers to "IsReadyObserver"
    readyObserver.setStandartObserver(&c1, &Counter::incValue);
    readyObserver.setStandartObserver( nullptr );
    readyObserver.setSingleTimeObserver(&c2, &Counter::incValue);

    // Check values before calling a signals
    ASSERT_EQ(c1.getValue(), 0); // for StandartObserver ("callOnReady")
    ASSERT_EQ(c2.getValue(), 0); // for SingleTimeObserver ("callOnceReady")

    // Set signal tracking for "IsReadyObserver"
    QSignalSpy spyObserver(&readyObserver, &IsReadyObserver::readyChanged);
    ASSERT_EQ(spyObserver.isValid(), true);
    spyObserver.clear();

    // Set signal tracking for "Proxy"
    QSignalSpy spyProxy(&proxy, &IPCProxy<InterfaceBase>::readyChanged );
    ASSERT_EQ(spyProxy.isValid(), true);
    spyProxy.clear();

    // Check that the signals on "IsReadyObserver" and "Proxy" are absent
    ASSERT_EQ(spyObserver.count(), 0);
    ASSERT_EQ(spyProxy.count(), 0);

    // Generate a some signals on the "Proxy" which is tracking on "IsReadyObserver"
    for (auto i=0; i<3; ++i) {
        proxy.readyChanged();
    }

    // Check the number of received signals on "Proxy"
    ASSERT_EQ(spyProxy.count(), 3);
    // Check the number of received signals on "IsReadyObserver"
    ASSERT_EQ(spyObserver.count(), 3);

    // Check values after signals call
    ASSERT_EQ(c1.getValue(), 3); // for StandartObserver ("callOnReady")
    ASSERT_EQ(c2.getValue(), 1); // for SingleTimeObserver ("callOnceReady")
}

TEST_F(IPCProxyTest, deleteObserverRun)
{
     // Checking if proxy is ready and signal exists and 1 from 3 observers with nullptr callback, all valid observer should be executed.
     // Set to Proxy in state "true" and set 3 observers, 1 from them with nullptr callback

    // Set state ready() of Proxy to enable
    proxy.setReadyTest(true);

    // Setting up an "IsReadyObserver" to watch out for signal in "Proxy"
    readyObserver.watch(&proxy, &IPCProxy<InterfaceBase>::readyChanged, &IPCProxy<InterfaceBase>::readyTest);
    // Set observers to "IsReadyObserver"
    const auto sso = readyObserver.setStandartObserver(&c1, &Counter::incValue);
    readyObserver.setSingleTimeObserver(&c2, &Counter::incValue);
    readyObserver.setStandartObserver(&c3, &Counter::addValue, 50);
    // Check values before calling a signals
    ASSERT_EQ(c1.getValue(), 0); // for StandartObserver ("callOnReady")
    ASSERT_EQ(c2.getValue(), 0); // for SingleTimeObserver ("callOnceReady")

    // Set signal tracking for "IsReadyObserver"
    QSignalSpy spyObserver(&readyObserver, &IsReadyObserver::readyChanged);
    ASSERT_EQ(spyObserver.isValid(), true);
    spyObserver.clear();

    // Set signal tracking for "Proxy"
    QSignalSpy spyProxy(&proxy, &IPCProxy<InterfaceBase>::readyChanged );
    ASSERT_EQ(spyProxy.isValid(), true);
    spyProxy.clear();

    // Check that the signals on "IsReadyObserver" and "Proxy" are absent
    ASSERT_EQ(spyObserver.count(), 0);
    ASSERT_EQ(spyProxy.count(), 0);

    // Clear observers
    readyObserver.delObserver(sso);
    ASSERT_EQ(readyObserver.count(), 2);

    // Generate a some signals on the "Proxy" which is tracking on "IsReadyObserver"
    for (auto i=0; i<3; ++i) {
        proxy.readyChanged();
    }

    // Check the number of received signals on "Proxy"
    ASSERT_EQ(spyProxy.count(), 3);
    // Check the number of received signals on "IsReadyObserver"
    ASSERT_EQ(spyObserver.count(), 3);

    // Check values after signals call
    ASSERT_EQ(c1.getValue(), 0); // for StandartObserver ("callOnReady")
    ASSERT_EQ(c2.getValue(), 1); // for SingleTimeObserver ("callOnceReady")
    ASSERT_EQ(c3.getValue(), 150); // for StandartObserver ("callOnReady")
}
TEST_F(IPCProxyTest, clearObserverRun)
{
     // Checking if proxy is ready and signal exists and 1 from 3 observers with nullptr callback, all valid observer should be executed.
     // Set to Proxy in state "true" and set 3 observers, 1 from them with nullptr callback

    // Set state ready() of Proxy to enable
    proxy.setReadyTest(true);

    // Setting up an "IsReadyObserver" to watch out for signal in "Proxy"
    readyObserver.watch(&proxy, &IPCProxy<InterfaceBase>::readyChanged, &IPCProxy<InterfaceBase>::readyTest);
    // Set observers to "IsReadyObserver"
    const auto sso = readyObserver.setStandartObserver(&c1, &Counter::incValue);
    readyObserver.setSingleTimeObserver(&c2, &Counter::incValue);

    // Check values before calling a signals
    ASSERT_EQ(c1.getValue(), 0); // for StandartObserver ("callOnReady")
    ASSERT_EQ(c2.getValue(), 0); // for SingleTimeObserver ("callOnceReady")

    // Set signal tracking for "IsReadyObserver"
    QSignalSpy spyObserver(&readyObserver, &IsReadyObserver::readyChanged);
    ASSERT_EQ(spyObserver.isValid(), true);
    spyObserver.clear();

    // Set signal tracking for "Proxy"
    QSignalSpy spyProxy(&proxy, &IPCProxy<InterfaceBase>::readyChanged );
    ASSERT_EQ(spyProxy.isValid(), true);
    spyProxy.clear();

    // Check that the signals on "IsReadyObserver" and "Proxy" are absent
    ASSERT_EQ(spyObserver.count(), 0);
    ASSERT_EQ(spyProxy.count(), 0);

    // Clear observers
    readyObserver.clear();
    ASSERT_EQ(readyObserver.count(), 0);

    // Generate a some signals on the "Proxy" which is tracking on "IsReadyObserver"
    for (auto i=0; i<3; ++i) {
        proxy.readyChanged();
    }

    // Check the number of received signals on "Proxy"
    ASSERT_EQ(spyProxy.count(), 3);
    // Check the number of received signals on "IsReadyObserver"
    ASSERT_EQ(spyObserver.count(), 3);

    // Check values after signals call
    ASSERT_EQ(c1.getValue(), 0); // for StandartObserver ("callOnReady")
    ASSERT_EQ(c2.getValue(), 0); // for SingleTimeObserver ("callOnceReady")
}
} // end namespace
