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

#include <QObject>
#include <QString>
#include <QMap>
#include <functional>
#include <QVector>
#include <QDebug>

namespace facelift {

template<typename Type, typename Key = QString>
class Registry {

    struct Listener {
        QPointer<QObject> context = nullptr;
        std::function<void()> function;
    };

public:

    Registry(QObject* owner)
    {
        m_owner = owner;
    }

    template<typename SourceObjectType, typename SourceSignalType>
    void addListener(const Key& key, SourceObjectType *receiver, void (SourceSignalType::*receiverMethod)()) const
    {
        auto& listeners = m_listeners[key];
        Listener e;
        e.context = receiver;
        e.function = [receiverMethod, receiver]() {
            (receiver->*receiverMethod)();
        };
        listeners.append(e);
        QObject::connect(receiver, &QObject::destroyed, m_owner, [this, key] (QObject *obj) {
            auto& elements = m_listeners[key];
            for (int i = 0; i < elements.size(); ++i) {
                if (elements[i].context == obj) {
                    elements.remove(i);
                    break;
                }
            }
            removeInvalidListeners(key);
        });
    }

    void notifyListeners(const Key& key) const
    {
        if (m_listeners.contains(key)) {
            const auto listeners = m_listeners[key];
            for (const auto& listener : listeners) {
                if (listener.context) {
                    listener.function();
                }
            }
            removeInvalidListeners(key);
        }
    }

    typename QMap<Key, Type>::iterator insert(const Key& key, Type value) {
        Q_ASSERT(!m_content.contains(key));
        auto iterator = m_content.insert(key, value);
        notifyListeners(key);
        return iterator;
    }

    bool remove(const Key& key) {
        bool isItemRemoved = m_content.remove(key);
        notifyListeners(key);
        return isItemRemoved;
    }

    bool contains(const Key& key) const
    {
        return m_content.contains(key);
    }

    QList<Key> keys() const
    {
        return m_content.keys();
    }

    const Type operator[](const Key& key) const
    {
        return m_content[key];
    }

    void setContent(const QMap<Key, Type> &content)
    {
        // TODO : optimize the code below if needed.
        // Note that this method is rarely called with non-empty content, so
        // the lack of efficiency of the following code is not critical.
        QVector<Key> modifiedItems;

        // find removed items
        for (auto & newKey : content.keys()) {
            auto contains = m_content.contains(newKey);
            if (!contains || (contains && m_content[newKey] != content[newKey])) {
                modifiedItems.append(newKey);
            }
        }

        // find new items
        for (auto & previousKey : m_content.keys()) {
            auto contains = content.contains(previousKey);
            if (!contains || (contains && m_content[previousKey] != content[previousKey])) {
                if (!modifiedItems.contains(previousKey)) {
                    modifiedItems.append(previousKey);
                }
            }
        }

        m_content = content;

        for (const auto & key : modifiedItems) {
            notifyListeners(key);
        }

    }

private:
    QMap<Key, Type> m_content;
    mutable QMap<Key, QVector<Listener>> m_listeners;  // "mutable" since adding a listener is not considered as a change
    QObject* m_owner = nullptr;

    void removeInvalidListeners(const Key& key) const
    {
        auto& listeners = m_listeners[key];
        for (int i = listeners.size() - 1; i >= 0; --i) {
            if (listeners[i].context == nullptr) {
                listeners.remove(i);
            }
        }
    }
};

}
