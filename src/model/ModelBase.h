/**
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

namespace facelift {

class ModelBase : public QObject
{
    Q_OBJECT

public:

    ModelBase();

    /**
     * See https://doc.qt.io/qt-5/qabstractitemmodel.html#dataChanged
     */
    Q_SIGNAL void dataChanged(int first, int last);

    /**
     * See https://doc.qt.io/qt-5/qabstractitemmodel.html#dataChanged
     */
    void dataChanged(int index)
    {
        dataChanged(index, index);
    }

    /**
     * See https://doc.qt.io/qt-5/qabstractitemmodel.html#beginInsertRows
     */
    Q_SIGNAL void beginInsertElements(int first, int last);

    /**
     * See https://doc.qt.io/qt-5/qabstractitemmodel.html#beginRemoveRows
     */
    Q_SIGNAL void beginRemoveElements(int first, int last);

    /**
     * See https://doc.qt.io/qt-5/qabstractitemmodel.html#beginMoveRows
     */
    Q_SIGNAL void beginMoveElements(int sourceFirst, int sourceLast, int destinationRow);

    /**
     * See https://doc.qt.io/qt-5/qabstractitemmodel.html#endMoveRows
     */
    Q_SIGNAL void endMoveElements();

    /**
     * See https://doc.qt.io/qt-5/qabstractitemmodel.html#endInsertRows
     */
    Q_SIGNAL void endInsertElements();

    /**
     * See https://doc.qt.io/qt-5/qabstractitemmodel.html#endRemoveRows
     */
    Q_SIGNAL void endRemoveElements();

    /**
     * See https://doc.qt.io/qt-5/qabstractitemmodel.html#beginResetModel
     */
    Q_SIGNAL void beginResetModel();

    /**
     * See https://doc.qt.io/qt-5/qabstractitemmodel.html#endResetModel
     */
    Q_SIGNAL void endResetModel();

    Q_SIGNAL void elementCountChanged();

    int size() const
    {
        return m_size;
    }

    void bindOtherModel(facelift::ModelBase *otherModel);

protected:
    void setSize(int size)
    {
        m_size = size;
    }

private:
    void onModelChanged();
    void applyNewSize();

    int m_size = 0;
    int m_previousElementCount = 0;
    int m_pendingSize = -1;

protected:
    bool m_resettingModel = false;

};


}
