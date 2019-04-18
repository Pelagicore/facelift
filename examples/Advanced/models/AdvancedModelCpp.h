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
        qDebug() << "Resetting model";

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
        qDebug() << "Get item at" << index;
        return m_items[index];
    }

    int getItemIndexByStructId(int id) const {
        for (int i = 0; i< m_items.size(); i++) {
            if (m_items[i].structId() == id) {
                return i;
            }
        }
        return INVALID_INDEX;
    }

    void moveItemDown(const MyStruct &item) override {
        qDebug() << "Deleting" << item;
        auto index = getItemIndexByStructId(item.structId());
        if ((index != INVALID_INDEX) && (index < m_items.size() - 1)) {
            emit m_theModel.beginMoveElements(index, index, index + 2);
            std::swap(m_items[index], m_items[index+1]);
            emit m_theModel.endMoveElements();
            qDebug() << "Moved down" << item;
        }
    }


    void moveItemUp(const MyStruct &item) override {
        qWarning() << "Deleting" << item;
        auto index = getItemIndexByStructId(item.structId());
        if ((index != INVALID_INDEX) && (index > 0)) {
            emit m_theModel.beginMoveElements(index, index, index - 1);
            std::swap(m_items[index], m_items[index-1]);
            emit m_theModel.endMoveElements();
            qDebug() << "Moved up" << item;
        }
    }

    void deleteModelItem(const MyStruct &item) override
    {
        qDebug() << "Deleting" << item;
        auto index = getItemIndexByStructId(item.structId());
        if (index != INVALID_INDEX) {
            emit m_theModel.beginRemoveElements(index, index);
            m_items.remove(index);
            emit m_theModel.endRemoveElements();
            qDebug() << "Deleted" << item;
        }
    }

    MyStruct newElement() {
        MyStruct s;
        s.setstructId(m_nextAvailableID++);
        s.setname(QString("entry %1").arg(s.structId()));
        return s;
    }

    void insertNewModelItemAfter(const MyStruct &item) override
    {
        qDebug() << "inserting" << item;
        auto index = getItemIndexByStructId(item.structId());
        if (index != INVALID_INDEX) {
            emit m_theModel.beginInsertElements(index, index);
            m_items.insert(index, newElement());
            emit m_theModel.endInsertElements();
            qDebug() << "Duplicated" << item;
        }
    }

    void renameModelItem(const MyStruct &item, const QString &name) override
    {
        qDebug() << "Renaming" << item;
        auto index = getItemIndexByStructId(item.structId());

        if (index != INVALID_INDEX) {
            m_items[index].setname(name);
            emit m_theModel.dataChanged(index);
            qDebug() << "Renamed" << item << "at index" << index;
        }
    }

private:
    int m_nextAvailableID = 0;
    QVector<MyStruct> m_items;
};
