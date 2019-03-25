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

#include "advanced/AdvancedModelImplementationBase.h"
#include <functional>

using namespace advanced;

class AdvancedModelImplementation : public AdvancedModelImplementationBase
{
    Q_OBJECT

    static constexpr int INVALID_INDEX = -1;

public:
    AdvancedModelImplementation(QObject *parent = nullptr) :
        AdvancedModelImplementationBase(parent)
    {
        resetModel();
    }

    void resetModel() override {
        qCWarning(LogGeneral) << "Resetting model";

        m_theModel.beginResetModel();

        m_nextAvailableID = 0;
        m_items.clear();

        int count = qrand() % 1000;

        int i = 0;
        for (; i < count; i++) {
            m_items.append(newElement());
        }

        m_theModel.reset(m_items.size(), std::bind(&AdvancedModelImplementation::getItem, this, std::placeholders::_1));
        m_theModel.endResetModel();
    }

    MyStruct getItem(int index)
    {
        qCWarning(LogGeneral) << "Get item at" << index;
        return m_items[index];
    }

    int getItemIndexById(int id) const {
        for (int i = 0; i< m_items.size(); i++) {
            if (m_items[i].id() == id) {
                return i;
            }
        }
        return INVALID_INDEX;
    }

    void moveItemDown(MyStruct item) override {
        qCWarning(LogGeneral) << "Deleting" << item;
        auto index = getItemIndexById(item.id());
        if ((index != INVALID_INDEX) && (index < m_items.size() - 1)) {
            emit m_theModel.beginMoveElements(index, index, index + 2);
            std::swap(m_items[index], m_items[index+1]);
            emit m_theModel.endMoveElements();
            qCWarning(LogGeneral) << "Moved down" << item;
        }
    }

    void moveItemUp(MyStruct item) override {
        qCWarning(LogGeneral) << "Deleting" << item;
        auto index = getItemIndexById(item.id());
        if ((index != INVALID_INDEX) && (index > 0)) {
            emit m_theModel.beginMoveElements(index, index, index - 1);
            std::swap(m_items[index], m_items[index-1]);
            emit m_theModel.endMoveElements();
            qCWarning(LogGeneral) << "Moved up" << item;
        }
    }

    void deleteModelItem(MyStruct item) override
    {
        qCWarning(LogGeneral) << "Deleting" << item;
        auto index = getItemIndexById(item.id());
        if (index != INVALID_INDEX) {
            emit m_theModel.beginRemoveElements(index, index);
            m_items.remove(index);
            emit m_theModel.endRemoveElements();
            qCWarning(LogGeneral) << "Deleted" << item;
        }
    }

    MyStruct newElement() {
        MyStruct s;
        s.setId(m_nextAvailableID++);
        s.setname(QString("entry %1").arg(s.id()));
        return s;
    }

    void insertNewModelItemAfter(MyStruct item) override
    {
        qCWarning(LogGeneral) << "inserting" << item;
        auto index = getItemIndexById(item.id());
        if (index != INVALID_INDEX) {
            emit m_theModel.beginInsertElements(index, index);
            m_items.insert(index, newElement());
            emit m_theModel.endInsertElements();
            qCWarning(LogGeneral) << "Duplicated" << item;
        }
    }

    void renameModelItem(MyStruct item, QString name) override
    {
        qCWarning(LogGeneral) << "Renaming" << item;
        auto index = getItemIndexById(item.id());

        if (index != INVALID_INDEX) {
            m_items[index].setname(name);
            emit m_theModel.dataChanged(index);
            qCWarning(LogGeneral) << "Renamed" << item << "at index" << index;
        }
    }

private:
    int m_nextAvailableID = 0;
    QVector<MyStruct> m_items;
};
