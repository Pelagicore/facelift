/*
 *   This is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 *   Please see the LICENSE file for details.
 */

#include <QDebug>

#include "Property.h"


class QMLFrontendBase :
    public QObject
{
    Q_OBJECT
};


class InterfaceImplementation :
    public QObject
{
    Q_OBJECT
};


class AddressBookQMLFrontend :
    public QMLFrontendBase
{

    Q_OBJECT

public:
    Q_PROPERTY(QString property1 READ property1)
    virtual QString property1() const = 0;

    Q_INVOKABLE virtual void do1() = 0;

};


class AddressBookExtendedQMLFrontend :
    public AddressBookQMLFrontend
{

    Q_OBJECT

public:
    Q_PROPERTY(QString property2 READ property1)
    virtual QString property2() const = 0;

    Q_INVOKABLE virtual void do2() = 0;

};


class AddressBookImplementation :
    public InterfaceImplementation
{

    Q_OBJECT

public:
    virtual QString property1() const = 0;
    virtual void do1() = 0;

    Q_SIGNAL void signal1();

};


class AddressBookExtendedImplementation :
    public AddressBookImplementation
{

    Q_OBJECT

public:
    virtual QString property2() const = 0;
    virtual void do2() = 0;

    Q_SIGNAL void signal2();

};



class AddressBookImplementationWithProperties :
    public AddressBookImplementation
{

public:
    QString property1() const
    {
        return m_property1;
    }

    Property<QString> m_property1;
};


class AddressBookExtendedImplementationWithProperties :
    public AddressBookImplementationWithProperties, public AddressBookExtendedImplementation
{

public:
    QString property1() const
    {
        return AddressBookImplementationWithProperties::property1();
    }

    QString property2() const
    {
        return m_property2;
    }

    Property<QString> m_property2;
};
