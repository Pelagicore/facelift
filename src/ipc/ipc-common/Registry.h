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

#include <QString>
#include <QMap>
#include <functional>
#include <QVector>

template<typename Type, typename Key = QString>
class Registry {
public:
    using ListenerFunction = std::function<void()>;

    template<typename SourceObjectType, typename SourceSignalType>
    void addListener(const Key& key, SourceObjectType *receiver, void (SourceSignalType::*receiverMethod)()) const
    {
        auto& listeners = m_listeners[key];
        QPointer<SourceObjectType> p(receiver);
        listeners.append([p, receiverMethod]() {
            if (p) {
                auto *receiver = p.data();
                (receiver->*receiverMethod)();
            }
        });
    }

    typename QMap<Key, Type>::iterator insert(const Key& key, Type value) {
        Q_ASSERT(!m_content.contains(key));
        auto iterator = m_content.insert(key, value);
        for (const auto& listener : m_listeners[key]) {
            listener();
        }
        return iterator;
    }

    bool remove(const Key& key) {
        bool isItemRemoved = m_content.remove(key);
        for (const auto& listener : m_listeners[key]) {
            listener();
        }
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
            if (m_listeners.contains(key)) {
                for (const auto& a : m_listeners[key]) {
                    a();
                }
            }
        }

    }

private:
    QMap<Key, Type> m_content;
    mutable QMap<Key, QVector<ListenerFunction>> m_listeners;  // "mutable" since adding a listener is not considered as a change

};
