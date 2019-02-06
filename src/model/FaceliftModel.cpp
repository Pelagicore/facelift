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

#include "FaceliftModel.h"
#include "ServiceRegistry.h"

#include <QTimer>

namespace facelift {

ServiceRegistry::~ServiceRegistry()
{
}

void ServiceRegistry::registerObject(InterfaceBase *i)
{
    m_objects.append(i);

    // Notify later since our object is not yet fully constructed at this point in time
    QTimer::singleShot(0, [this, i] () {
        emit objectRegistered(i);
    });
}

ServiceRegistry &ServiceRegistry::instance()
{
    static ServiceRegistry reg;
    return reg;
}

ModelBase::ModelBase()
{
    QObject::connect(this, &facelift::ModelBase::beginInsertElements, this, [this] (int first, int last) {
        Q_ASSERT(m_pendingSize == -1);
        m_pendingSize = size() + last - first + 1;
    });
    QObject::connect(this, &facelift::ModelBase::beginRemoveElements, this, [this] (int first, int last) {
        Q_ASSERT(m_pendingSize == -1);
        m_pendingSize = size() - last + first - 1;
    });
    QObject::connect(this, &facelift::ModelBase::endRemoveElements, this, &ModelBase::applyNewSize);
    QObject::connect(this, &facelift::ModelBase::endInsertElements, this, &ModelBase::applyNewSize);
    QObject::connect(this, &facelift::ModelBase::beginResetModel, this, [this] () {
        m_resettingModel = true;
    });
    QObject::connect(this, &facelift::ModelBase::endResetModel, this, [this] () {
        m_resettingModel = false;
    });

    QObject::connect(this, &ModelBase::endResetModel, this, &ModelBase::onModelChanged);
    QObject::connect(this, &ModelBase::endInsertElements, this, &ModelBase::onModelChanged);
    QObject::connect(this, &ModelBase::endRemoveElements, this, &ModelBase::onModelChanged);
}

void ModelBase::bindOtherModel(facelift::ModelBase *otherModel) {
    QObject::connect(otherModel, &facelift::ModelBase::beginInsertElements, this, &facelift::ModelBase::beginInsertElements);
    QObject::connect(otherModel, &facelift::ModelBase::endInsertElements, this, &facelift::ModelBase::endInsertElements);
    QObject::connect(otherModel, &facelift::ModelBase::beginRemoveElements, this, &facelift::ModelBase::beginRemoveElements);
    QObject::connect(otherModel, &facelift::ModelBase::endRemoveElements, this, &facelift::ModelBase::endRemoveElements);
    QObject::connect(otherModel, &facelift::ModelBase::beginResetModel, this, &facelift::ModelBase::beginResetModel);
    QObject::connect(otherModel, &facelift::ModelBase::endResetModel, this, [this, otherModel] () {
        setSize(otherModel->size());
    });
    QObject::connect(otherModel, &facelift::ModelBase::endResetModel, this, &facelift::ModelBase::endResetModel);
    QObject::connect(otherModel, static_cast<void (facelift::ModelBase::*)(int,int)>(&facelift::ModelBase::dataChanged),
            (facelift::ModelBase*)this, static_cast<void (facelift::ModelBase::*)(int,int)>(&facelift::ModelBase::dataChanged));
}

void ModelBase::applyNewSize()
{
    setSize(m_pendingSize);
    m_pendingSize = -1;
}

void ModelBase::onModelChanged()
{
    if (m_previousElementCount != size()) {
        m_previousElementCount = size();
        emit elementCountChanged();
    }
}

void InterfaceBase::init(const QString &interfaceName)
{
    m_interfaceName = interfaceName;
    facelift::ServiceRegistry::instance().registerObject(this);
}

}
