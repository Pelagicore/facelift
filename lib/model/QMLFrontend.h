/*
 *   This is part of the QMLCppAPI project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 *   Please see the LICENSE file for details.
 */

#pragma once

#include <QObject>
#include <QDebug>
#include <QQmlEngine>
#include <QtQml>
#include <functional>


class QMLFrontend :
    public QObject
{

    Q_OBJECT

public:
    QMLFrontend(QObject *parent = nullptr) :
        QObject(parent)
    {
    }

    Q_PROPERTY(QObject * provider READ provider CONSTANT)
    virtual QObject * provider() = 0;

    Q_PROPERTY(QString implementationID READ implementationID CONSTANT)
    virtual const QString &implementationID() = 0;

};

template<typename ProviderType, typename QMLType>
class TQMLFrontend :
    public QMLType
{

public:
    TQMLFrontend() :
        QMLType()
    {
        this->init(m_provider);
    }

    QObject *provider()
    {
        qWarning() << "Accessing private provider implementation object";
        return m_provider.impl();
    }

    virtual const QString &implementationID()
    {
        return m_provider.implementationID();
    }

    ProviderType m_provider;

};

template<typename ProviderType>
void registerQmlComponent(const char *uri, const char *name = ProviderType::QMLFrontendType::INTERFACE_NAME)
{
    ProviderType::registerTypes(uri);
    qmlRegisterType<TQMLFrontend<ProviderType, typename ProviderType::QMLFrontendType> >(uri, 1, 0, name);
}


class ModelListModelBase :
    public QAbstractListModel
{

    Q_OBJECT

public:
    typedef size_t (QObject::*SizeGetterFunction)();

    void notifyModelChanged()
    {
        auto previousRowCount = m_rowCount;
        auto newRowCount = (m_provider->*m_sizeGetter)();

        if (newRowCount > previousRowCount) {
            // And a beginInsertRows() & endInsertRows() for the new items
            beginInsertRows(QModelIndex(), previousRowCount, newRowCount - 1);
            syncWithProvider();
            endInsertRows();
        } else {
            // And a beginInsertRows() & endInsertRows() for the new items
            beginRemoveRows(QModelIndex(), newRowCount, previousRowCount - 1);
            syncWithProvider();
            endRemoveRows();
        }

        // Trigger "dataChanged()" for the items which were existing previously
        QModelIndex previousTopLeft = index(0, 0);
        QModelIndex previousBottomRight = index(m_rowCount - 1, 0);
        dataChanged(previousTopLeft, previousBottomRight);

    }

    void syncWithProvider()
    {
        m_rowCount = (m_provider->*m_sizeGetter)();
    }

protected:
    size_t m_rowCount = 0;
    SizeGetterFunction m_sizeGetter;
    QObject *m_provider;

};

template<typename ElementType>
class ModelListModel :
    public ModelListModelBase
{
public:
    typedef ElementType (QObject::*ElementGetterFunction)(size_t);

    ModelListModel()
    {
    }

    QHash<int, QByteArray> roleNames() const override
    {
        QHash<int, QByteArray> roles;
        roles[1000] = "modelData";
        return roles;
        //        return ElementType::roleNames_(ElementType::FIELD_NAMES);
    }

    int rowCount(const QModelIndex &index) const override
    {
        Q_UNUSED(index);
        return m_rowCount;
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        Q_UNUSED(role);
        qDebug() << "Geettt " << index.row();
        auto element = (m_provider->*m_elementGetter)(index.row());
        return QVariant::fromValue(element);
    }

    template<typename ProviderType>
    void init(QObject *ownerObject, void (ProviderType::*changeSignal)(), size_t (ProviderType::*sizeGetter)(),
            ElementType (ProviderType::*elementGetter)(size_t))
    {
        m_provider = ownerObject;
        m_sizeGetter = (SizeGetterFunction) sizeGetter;
        m_elementGetter = (ElementGetterFunction) elementGetter;
        syncWithProvider();
    }

private:
    ElementGetterFunction m_elementGetter;

};
