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

#pragma once

#include <QObject>
#include <QAbstractListModel>

#include "FaceliftModel.h"

namespace facelift {

class IPCAttachedPropertyFactory;
class IPCServiceAdapterBase;

class ModelListModelBase : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    typedef size_t (QObject::*SizeGetterFunction)();

    Q_SIGNAL void countChanged();

    ModelListModelBase();

    /**
     * Return the element at the given row index
     */
    Q_INVOKABLE QVariant get(int rowIndex) const;

    void setModelProperty(facelift::ModelBase &property);

    int rowCount(const QModelIndex &index = QModelIndex()) const override;

    QHash<int, QByteArray> roleNames() const override;

private:

    void onBeginResetModel();

    void onEndResetModel();

    void onBeginInsertElements(int first, int last);

    void onEndInsertElements();

    void onBeginMoveElements(int sourceFirstIndex, int sourceLastIndex, int destinationIndex);

    void onEndMoveElements();

    void onBeginRemoveElements(int first, int last);

    void onEndRemoveElements();

    void onDataChanged(int first, int last);

protected:
    facelift::ModelBase *m_property = nullptr;

};


}
