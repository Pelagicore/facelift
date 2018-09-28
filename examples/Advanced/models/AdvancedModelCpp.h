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

#include "advanced/AdvancedModelPropertyAdapter.h"
#include <functional>
#include <QStringBuilder>

using namespace advanced;

class AdvancedModelCpp : public AdvancedModelPropertyAdapter
{
    Q_OBJECT

public:
    AdvancedModelCpp(QObject *parent = nullptr) :
        AdvancedModelPropertyAdapter(parent)
    {
        resetModel();
    }

    void resetModel() override {
        qWarning() << "Resetting model";

        m_theModel.beginResetModel();

        m_renamedItems.clear();
        m_items.clear();

        int count = qrand() % 1000;

        int i = 0;
        for (; i < count; i++)
            m_items.append(i);

        m_nextAvailableID = i;

        m_theModel.reset(m_items.size(), std::bind(&AdvancedModelCpp::getItem, this, std::placeholders::_1));
        m_theModel.endResetModel();
    }

    MyStruct getItem(int index)
    {
        qWarning() << "Get item at" << index;
        const auto &itemId = m_items[index];
        MyStruct entry;
        QString s = "entry ";
        s += QString::number(itemId);
        entry.setname(m_renamedItems.contains(itemId) ? m_renamedItems[itemId] : s);
        entry.setenabled((itemId % 2) == 0);
        entry.setId(itemId);
        return entry;
    }

    void deleteModelItem(MyStruct item) override
    {
        qWarning() << "Deleting" << item;
        auto index = m_items.indexOf(item.id());
        if (index != -1) {
            emit m_theModel.beginRemoveElements(index, index);
            m_items.remove(index);
            emit m_theModel.endRemoveElements();
            qWarning() << "Deleted" << item;
        }
    }

    void insertNewModelItemAfter(MyStruct item) override
    {
        qWarning() << "inserting" << item;
        auto index = m_items.indexOf(item.id());
        if (index != -1) {
            emit m_theModel.beginInsertElements(index, index);
            m_items.insert(index, m_nextAvailableID++);
            emit m_theModel.endInsertElements();
            qWarning() << "Duplicated" << item;
        }
    }

    void renameModelItem(MyStruct item, QString name) override
    {
        qWarning() << "Renaming" << item;
        auto elementId = item.id();
        m_renamedItems[elementId] = name;
        auto index = m_items.indexOf(elementId);
        if (index != -1) {
            emit m_theModel.dataChanged(index);
            qWarning() << "Renamed" << item << m_renamedItems << "index" << index;
        }
    }

private:
    QVector<int> m_items;
    QMap<int, QString> m_renamedItems;
    int m_nextAvailableID = 0;
};
