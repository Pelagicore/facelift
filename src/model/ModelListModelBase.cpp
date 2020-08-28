/**********************************************************************
**
** Copyright (C) 2020 Luxoft Sweden AB
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

#include "ModelListModelBase.h"
#include "ServiceRegistry.h"

namespace facelift {


QVariant ModelListModelBase::get(int rowIndex) const {
    return data(index(rowIndex));
}

void ModelListModelBase::setModelProperty(facelift::ModelBase &property)
{
    beginResetModel();
    m_property = &property;
    QObject::connect(m_property, &facelift::ModelBase::beginInsertElements, this, &ModelListModelBase::onBeginInsertElements);
    QObject::connect(m_property, &facelift::ModelBase::endInsertElements, this, &ModelListModelBase::onEndInsertElements);
    QObject::connect(m_property, &facelift::ModelBase::beginRemoveElements, this, &ModelListModelBase::onBeginRemoveElements);
    QObject::connect(m_property, &facelift::ModelBase::endRemoveElements, this, &ModelListModelBase::onEndRemoveElements);
    QObject::connect(m_property, &facelift::ModelBase::beginMoveElements, this, &ModelListModelBase::onBeginMoveElements);
    QObject::connect(m_property, &facelift::ModelBase::endMoveElements, this, &ModelListModelBase::onEndMoveElements);
    QObject::connect(m_property, &facelift::ModelBase::beginResetModel, this, &ModelListModelBase::onBeginResetModel);
    QObject::connect(m_property, &facelift::ModelBase::endResetModel, this, &ModelListModelBase::onEndResetModel);
    QObject::connect(m_property, static_cast<void (facelift::ModelBase::*)(int, int)>(&facelift::ModelBase::dataChanged), this,
            &ModelListModelBase::onDataChanged);

    endResetModel();
}

void ModelListModelBase::onBeginResetModel()
{
    beginResetModel();
}

void ModelListModelBase::onEndResetModel()
{
    endResetModel();
}

void ModelListModelBase::onBeginMoveElements(int sourceFirstIndex, int sourceLastIndex, int destinationIndex)
{
    auto isOK = beginMoveRows(QModelIndex(), sourceFirstIndex, sourceLastIndex, QModelIndex(), destinationIndex);
    if (!isOK)
        qFatal("Invalid move operation");
}

void ModelListModelBase::onEndMoveElements()
{
    endMoveRows();
}

void ModelListModelBase::onBeginInsertElements(int first, int last)
{
    beginInsertRows(QModelIndex(), first, last);
}

void ModelListModelBase::onEndInsertElements()
{
    endInsertRows();
}

void ModelListModelBase::onBeginRemoveElements(int first, int last)
{
    beginRemoveRows(QModelIndex(), first, last);
}

void ModelListModelBase::onEndRemoveElements()
{
    endRemoveRows();
}

void ModelListModelBase::onDataChanged(int first, int last)
{
    dataChanged(createIndex(first, 0), createIndex(last, 0));
}

int ModelListModelBase::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return m_property->size();
}

QHash<int, QByteArray> ModelListModelBase::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::UserRole] = "modelData";
    return roles;
}

ModelListModelBase::ModelListModelBase() {
    QObject::connect(this, &QAbstractItemModel::rowsInserted, this, &ModelListModelBase::countChanged);
    QObject::connect(this, &QAbstractItemModel::rowsRemoved, this, &ModelListModelBase::countChanged);
    QObject::connect(this, &QAbstractItemModel::modelReset, this, &ModelListModelBase::countChanged);
}


}
