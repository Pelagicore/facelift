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
#include <QtDBus>
#include "ipc-common.h"

#if defined(FaceliftIPCCommonLib_LIBRARY)
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_IMPORT
#endif

namespace facelift {


template<typename IPCAdapterType, typename ModelDataType>
class IPCAdapterModelPropertyHandler
{

public:
    IPCAdapterModelPropertyHandler(IPCAdapterType &adapter) : m_adapter(adapter)
    {
    }

    void connectModel(const QString& modelPropertyName, facelift::Model<ModelDataType> &model)
    {
        m_model = &model;
        QObject::connect(m_model, static_cast<void (facelift::ModelBase::*)(int, int)>
            (&facelift::ModelBase::dataChanged), &m_adapter, [this, modelPropertyName] (int first, int last) {
            QList<ModelDataType> changedItems;
            for (int i = first ; i <= last ; i++) {
                changedItems.append(m_model->elementAt(i));
            }
            m_adapter.sendSignal("ModelUpdateEventDataChanged", modelPropertyName, first, changedItems);
        });
        QObject::connect(m_model, &facelift::ModelBase::beginRemoveElements, &m_adapter, [this] (int first, int last) {
            m_removeFirst = first;
            m_removeLast = last;
        });
        QObject::connect(m_model, &facelift::ModelBase::endRemoveElements, &m_adapter, [this, modelPropertyName] () {
            Q_ASSERT(m_removeFirst != UNDEFINED);
            Q_ASSERT(m_removeLast != UNDEFINED);
            m_adapter.sendSignal("ModelUpdateEventRemove", modelPropertyName, m_removeFirst, m_removeLast);
            m_removeFirst = UNDEFINED;
            m_removeLast = UNDEFINED;
        });
        QObject::connect(m_model, &facelift::ModelBase::beginMoveElements, &m_adapter, [this] (int sourceFirstIndex, int sourceLastIndex, int destinationIndex) {
            m_moveSourceFirstIndex = sourceFirstIndex;
            m_moveSourceLastIndex = sourceLastIndex;
            m_moveDestinationIndex = destinationIndex;
        });
        QObject::connect(m_model, &facelift::ModelBase::endMoveElements, &m_adapter, [this, modelPropertyName] () {
            Q_ASSERT(m_moveSourceFirstIndex != UNDEFINED);
            Q_ASSERT(m_moveSourceLastIndex != UNDEFINED);
            Q_ASSERT(m_moveDestinationIndex != UNDEFINED);
            m_adapter.sendSignal("ModelUpdateEventMove", modelPropertyName, m_moveSourceFirstIndex, m_moveSourceLastIndex, m_moveDestinationIndex);
            m_moveSourceFirstIndex = UNDEFINED;
            m_moveSourceLastIndex = UNDEFINED;
            m_moveDestinationIndex = UNDEFINED;
        });
        QObject::connect(m_model, &facelift::ModelBase::beginInsertElements, &m_adapter, [this, modelPropertyName] (int first, int last) {
            m_insertFirst = first;
            m_insertLast = last;
        });
        QObject::connect(m_model, &facelift::ModelBase::endInsertElements, &m_adapter, [this, modelPropertyName] () {
            Q_ASSERT(m_insertFirst != UNDEFINED);
            Q_ASSERT(m_insertLast != UNDEFINED);
            m_adapter.sendSignal("ModelUpdateEventInsert", modelPropertyName, m_insertFirst, m_insertLast);
            m_insertFirst = UNDEFINED;
            m_insertLast = UNDEFINED;
        });
        QObject::connect(m_model, &facelift::ModelBase::beginResetModel, &m_adapter, [this, modelPropertyName] () {
            m_resettingModel = true;
        });
        QObject::connect(m_model, &facelift::ModelBase::endResetModel, &m_adapter, [this, modelPropertyName] () {
            Q_ASSERT(m_resettingModel);
            m_adapter.sendSignal("ModelUpdateEventReset", modelPropertyName, m_model->size());
            m_resettingModel = false;
        });
    }

    void handleModelRequest(typename IPCAdapterType::InputIPCMessage &requestMessage, typename IPCAdapterType::OutputIPCMessage &replyMessage)
    {
        QListIterator<QVariant> argumentsIterator(requestMessage.arguments());
        int first = (argumentsIterator.hasNext() ? qdbus_cast<int>(argumentsIterator.next()): int());
        int last = (argumentsIterator.hasNext() ? qdbus_cast<int>(argumentsIterator.next()): int());
        QList<ModelDataType> list;

        // Make sure we do not request items which are out of range
        first = qMax(first, 0);
        last = qMin(last, m_model->size() - 1);

        for (int i = first; i <= last; ++i) {
            list.append(m_model->elementAt(i));
        }

        replyMessage << QVariant::fromValue(first);
        replyMessage << QVariant::fromValue(list);
    }

private:
    static constexpr int UNDEFINED = -1;
    IPCAdapterType &m_adapter;
    facelift::Model<ModelDataType> *m_model = nullptr;
    int m_removeFirst = UNDEFINED;
    int m_removeLast = UNDEFINED;
    int m_insertFirst = UNDEFINED;
    int m_insertLast = UNDEFINED;
    int m_moveSourceFirstIndex = UNDEFINED;
    int m_moveSourceLastIndex = UNDEFINED;
    int m_moveDestinationIndex = UNDEFINED;
    bool m_resettingModel = false;
};

}
