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


class QMLFrontend :
    public QObject
{

    Q_OBJECT

public:
    QMLFrontend(QObject *parent = nullptr) :
        QObject(parent)
    {
    }

    Q_PROPERTY(QString implementationID READ implementationID CONSTANT);
    Q_PROPERTY(QObject * provider READ provider CONSTANT);
    virtual QObject *provider() = 0;

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
