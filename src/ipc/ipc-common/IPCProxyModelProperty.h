/**********************************************************************
**
** Copyright (C) 2019 Luxoft Sweden AB
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

namespace facelift {

template<typename IPCProxyType, typename ModelDataType>
class IPCProxyModelProperty : public facelift::ModelProperty<ModelDataType>
{

    using MemberID = typename IPCProxyType::MemberIDType;

public:
    IPCProxyModelProperty(IPCProxyType &proxy) : m_proxy(proxy), m_cache(PREFETCH_ITEM_COUNT * 10)
    {
    }

    void handleSignal(typename IPCProxyType::InputIPCMessage &msg)
    {
        ModelUpdateEvent event;
        m_proxy.deserializeValue(msg, event);
        switch (event) {

        case ModelUpdateEvent::DataChanged:
        {
            int first;
            QList<ModelDataType> list;
            m_proxy.deserializeValue(msg, first);
            m_proxy.deserializeValue(msg, list);
            int last = first + list.size() - 1;
            for (int i = first; i <= last; ++i) {
                m_cache.insert(i, list.at(i - first));
            }
            emit this->dataChanged(first, last);
        } break;

        case ModelUpdateEvent::Insert:
        {
            int first, last;
            m_proxy.deserializeValue(msg, first);
            m_proxy.deserializeValue(msg, last);
            emit this->beginInsertElements(first, last);
            clear(); // TODO: insert elements in cache without clear()
            emit this->endInsertElements();
        } break;

        case ModelUpdateEvent::Remove:
        {
            int first, last;
            m_proxy.deserializeValue(msg, first);
            m_proxy.deserializeValue(msg, last);
            emit this->beginRemoveElements(first, last);
            m_cache.clear(); // TODO: remove elements from cache without clear()
            emit this->endRemoveElements();
        } break;

        case ModelUpdateEvent::Move:
        {
            int sourceFirstIndex, sourceLastIndex, destinationIndex;
            m_proxy.deserializeValue(msg, sourceFirstIndex);
            m_proxy.deserializeValue(msg, sourceLastIndex);
            m_proxy.deserializeValue(msg, destinationIndex);
            emit this->beginMoveElements(sourceFirstIndex, sourceLastIndex, destinationIndex);
            m_cache.clear(); // TODO: move elements in cache without clear()
            emit this->endMoveElements();
        } break;

        case ModelUpdateEvent::Reset:
        {
            emit this->beginResetModel();
            int size;
            m_proxy.deserializeValue(msg, size);
            this->setSize(size);
            clear();
            emit this->endResetModel();
        } break;

        }
    }

    void clear()
    {
        m_cache.clear();
        m_itemsRequestedFromServer.clear();
        m_itemsRequestedLocally.clear();
    }

    ModelDataType modelData(const MemberID &requestMemberID, int row)
    {
        if (!m_cache.exists(row)) {
            if (m_proxy.isSynchronous()) {

                int first = row > PREFETCH_ITEM_COUNT ? row - PREFETCH_ITEM_COUNT : 0;
                int last = row < this->size() - PREFETCH_ITEM_COUNT ? row + PREFETCH_ITEM_COUNT : this->size() - 1;

                while (m_cache.exists(first) && first < last) {
                    ++first;
                }
                while (m_cache.exists(last) && last > first) {
                    --last;
                }

                std::tuple<int, QList<ModelDataType>> requestResult;
                m_proxy.ipc()->sendMethodCallWithReturn(requestMemberID, requestResult, first, last);

                first = std::get<0>(requestResult);
                auto &list = std::get<1>(requestResult);
                last = first + list.size() - 1;

                for (int i = first; i <= last; ++i) {
                    m_cache.insert(i, list.at(i - first));
                }
            } else {
                // Request items asynchronously and return a default item for now
                requestItemsAsync(requestMemberID, row);
                m_itemsRequestedLocally.append(row); // Remind that we delivered an invalid item for this index
            }
        }

        // Prefetch next items
        int nextIndex = std::min(this->size(), row + PREFETCH_ITEM_COUNT);
        if (!m_cache.exists(nextIndex) && !m_itemsRequestedFromServer.contains(nextIndex)) {
            requestItemsAsync(requestMemberID, nextIndex);
        }

        // Prefetch previous items
        int previousIndex = std::max(0, row - PREFETCH_ITEM_COUNT);
        if (!m_cache.exists(nextIndex) && !m_itemsRequestedFromServer.contains(previousIndex)) {
            requestItemsAsync(requestMemberID, previousIndex);
        }

        return m_cache.exists(row) ? m_cache.get(row) : ModelDataType {};
    }

    /**
     * Request the items around the given index.
     */
    void requestItemsAsync(const MemberID &requestMemberID, int index)
    {
        // Find the first index which we should request, given what we already have in our cache
        int first = std::max(0, index - PREFETCH_ITEM_COUNT);
        while ((m_cache.exists(first) || m_itemsRequestedFromServer.contains(first)) && (first < this->size())) {
            ++first;
        }

        if ((first - index < PREFETCH_ITEM_COUNT) && (first != this->size())) {  // We don't request anything if the first index is outside the window
            int last = std::min(first + PREFETCH_ITEM_COUNT * 2, this->size() - 1);   // We query at least

            // Do not request the items from the end of the window, which we already have in our cache
            while ((m_cache.exists(last) || m_itemsRequestedFromServer.contains(last)) && (last >= first)) {
                --last;
            }

            if (first <= last) {
                m_proxy.ipc()->sendAsyncMethodCall(requestMemberID, facelift::AsyncAnswer<std::tuple<int, QList<ModelDataType>>>(&m_proxy, [this](std::tuple<int, QList<ModelDataType>> result) {
                        //                    qCDebug(LogIpc) << "Received model items " << first << "-" << last;
                        auto & first = std::get<0>(result);
                        auto & list = std::get<1>(result);
                        auto last = first + list.size() - 1;
                        for (int i = first; i <= last; ++i) {
                            auto &newItem = list[i - first];
                            if (!((m_cache.exists(i)) && (newItem == m_cache.get(i)))) {
                                m_cache.insert(i, newItem);
                                if (m_itemsRequestedLocally.contains(i)) {
                                    this->dataChanged(i);
                                    m_itemsRequestedLocally.removeAll(i);
                                }
                            }
                            m_itemsRequestedFromServer.removeAll(i);
                        }
                    }), first, last);
                for (int i = first; i <= last; ++i) {
                    m_itemsRequestedFromServer.append(i);
                }
            }
        }
    }

private:
    static constexpr int PREFETCH_ITEM_COUNT = 12;        // fetch 25 items around requested one

    IPCProxyType &m_proxy;
    facelift::MostRecentlyUsedCache<int, ModelDataType> m_cache;
    QList<int> m_itemsRequestedFromServer;
    QList<int> m_itemsRequestedLocally;
};

}
