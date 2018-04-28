/*
 *   This file is part of the FaceLift project
 *   Copyright (C) 2018 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the <license name> licence.
 *   Please see the LICENSE file for details.
 */
#pragma once

#include "advanced/AdvancedModelPropertyAdapter.h"
#include <functional>
#include <QStringBuilder>

using namespace advanced;

class AdvancedModelCpp :
    public AdvancedModelPropertyAdapter
{

    Q_OBJECT

public:
    AdvancedModelCpp(QObject *parent = nullptr) :
        AdvancedModelPropertyAdapter(parent)
    {
        for (int i = 0; i < 100; i++) {
            m_items.append(i);
            m_nextAvailableID++;
        }

        m_theModel.setSize(m_items.size());
        m_theModel.setGetter([this](int index) {
            return getItem(index);
        });
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
            m_theModel.beginRemoveElements(index, index);
            m_items.remove(index);
            m_theModel.endRemoveElements();
            qWarning() << "Deleted" << item;
        }
    }

    void insertNewModelItemAfter(MyStruct item) override
    {
        qWarning() << "inserting" << item;
        auto index = m_items.indexOf(item.id());
        if (index != -1) {
            m_theModel.beginInsertElements(index, index);
            m_items.insert(index, m_nextAvailableID++);
            m_theModel.endInsertElements();
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
            m_theModel.dataChanged(index);
            qWarning() << "Renamed" << item << m_renamedItems << "index" << index;
        }
    }

private:
    QVector<int> m_items;
    QMap<int, QString> m_renamedItems;
    int m_nextAvailableID = 0;

};
