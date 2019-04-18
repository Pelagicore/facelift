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

#include "tests/models/ModelInterfaceImplementationBase.h"
#include <functional>
#include <QStringBuilder>

using namespace tests::models;


class ModelInterfaceImplementation : public ModelInterfaceImplementationBase
{
public:
    ModelInterfaceImplementation(QObject *parent = nullptr) :
        ModelInterfaceImplementationBase(parent)
    {
        int i = 0;
        for (; i < 100; i++)
            m_items.append(i);

        m_nextAvailableID = i;

        m_theModel.reset(m_items.size(),
                         std::bind(&ModelInterfaceImplementation::getItem, this, std::placeholders::_1));
    }

    TestStruct getItem(int index)
    {
        const auto &itemId = m_items[index];
        TestStruct entry;
        QString s = "entry ";
        s += QString::number(itemId);
        entry.setname(s);
        entry.setenabled((itemId % 2) == 0);
        return entry;
    }

    void deleteModelItems(int first, int last) override
    {
        Q_ASSERT(first >= 0);
        Q_ASSERT(last >= 0);
        Q_ASSERT(last < m_items.count());
        QTimer::singleShot(0, [this, first, last] () {
            emit m_theModel.beginRemoveElements(first, last);
            m_items.remove(first, last - first + 1);
            emit m_theModel.endRemoveElements();
        });
    }

    void insertNewModelItems(int first, int last) override
    {
        QTimer::singleShot(0, [this, first, last] () {
            emit m_theModel.beginInsertElements(first, last);
            m_items.insert(first, last - first + 1, m_nextAvailableID++);
            emit m_theModel.endInsertElements();
        });
    }

    void renameModelItem(int first, int last, int serial) override
    {
        for (int i = first; i <= last; ++i)
            m_items[i] = serial;

        emit m_theModel.dataChanged(first, last);
    }

private:
    QVector<int> m_items;
    int m_nextAvailableID = 0;
};
