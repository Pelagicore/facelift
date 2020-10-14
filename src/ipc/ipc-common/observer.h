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

#include <memory>
#include <QVector>
#include <QObject>
#include <algorithm>
#include <functional>

namespace facelift {

class Observer : public QObject
{
    Q_OBJECT
};

// Single-time observer which will unregister itself when done
template<typename T>
class SingleTimeObserver : public Observer
{
    T m_function;

public:
    explicit SingleTimeObserver(T function) : m_function{function} {}
    ~SingleTimeObserver() = default;

    void IsReadyObserver(const std::shared_ptr<QMetaObject::Connection> &connection) {
        if (m_function != nullptr) {
            m_function();
        }
        QObject::disconnect(*connection);
    }
};

// Standard observer which will work for each signal
template<typename T>
class StandartObserver : public Observer
{
    T m_function;

public:
    explicit StandartObserver(T function) : m_function{function} {}
    ~StandartObserver() = default;

    void IsReadyObserver(const std::shared_ptr<QMetaObject::Connection> &) {
        if (m_function != nullptr) {
            m_function();
        }
    }
};

class IsReadyObserver: public QObject
{
    Q_OBJECT
    QVector< std::shared_ptr< Observer > > m_observers{};
    bool isReady{};

public:
    IsReadyObserver() {}

    // Set the object, signal to watch for
    template<typename T, typename S>
    void watch(T const *object, S signal)
    {
         QObject::connect(object, std::move(signal), this, &IsReadyObserver::onReadyChanged);
         isReady = true;
    }

    // Set the object, signal, function to watch for
    template<typename T, typename S, typename F>
    void watch(T const *object, S signal, F function)
    {
        QObject::connect(object, std::move(signal), object, [this, function, object](){
             std::function<bool()> functor { std::bind(function, object) };
             if (functor != nullptr) {
                 isReady = functor();
             }
         });
         QObject::connect(object, std::move(signal), this, &IsReadyObserver::onReadyChanged);
    }

    // Set observer
    template <typename T>
    void setObserver(const std::shared_ptr<T> &observer)
    {
        Q_ASSERT (observer != nullptr);
        m_observers.push_back(observer);
        auto connection = std::make_shared<QMetaObject::Connection>();
        *connection = QObject::connect(this, &IsReadyObserver::readyChanged, observer.get(), [wp_observer = std::weak_ptr<T>(observer), connection]() {
            auto sp_observer = wp_observer.lock();
            if (sp_observer){
                sp_observer->IsReadyObserver( connection );
            }
        });
    }

    // Delete observer
    void delObserver(const std::weak_ptr<Observer> &observer)
    {
        m_observers.erase( std::remove(m_observers.begin(), m_observers.end(), std::shared_ptr< Observer >(observer)), m_observers.end() );
    }

    // Clear all observers
    void clear()
    {
        m_observers.clear();
    }

    // Get count observers
    auto count()
    {
        return getObservers().size();
    }

    // Set "callOnReady" observer
    template<typename T>
    const std::weak_ptr<Observer> setStandartObserver(T function)
    {
        auto observer = std::make_shared< StandartObserver < std::function<void()> > > ( function );
        setObserver( observer );
        return observer;
    }

    // Set "callOnReady" observer
    template<typename T, typename F, typename ... Args>
    const std::weak_ptr<Observer> setStandartObserver(T object, F function, Args&& ... args)
    {
        auto observer = std::make_shared< StandartObserver < std::function<void()> > > ( std::bind(function, object, std::forward<Args>(args) ...) );
        setObserver( observer );
        return observer;
    }

    // Set "callOnceReady" observer
    template<typename T>
    const std::weak_ptr<Observer> setSingleTimeObserver(T function)
    {
        auto observer = std::make_shared< SingleTimeObserver < std::function<void()> > > ( function );
        setObserver( observer );
        return observer;
    }

    // Set "callOnceReady" observer
    template<typename T, typename F, typename ... Args>
    const std::weak_ptr<Observer> setSingleTimeObserver(T object, F function, Args&& ... args)
    {
        auto observer = std::make_shared< SingleTimeObserver < std::function<void()> > > (  std::bind(function, object, std::forward<Args>(args) ...) );
        setObserver( observer );
        return observer;
    }

    // Get observers
    const QVector< std::shared_ptr<Observer> > &getObservers() const
    {
        return m_observers;
    }

    Q_SIGNAL void readyChanged();

    void onReadyChanged()
    {
        if (isReady) {
            emit readyChanged();
        }
    }
};

}
