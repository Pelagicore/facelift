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

#include "FaceliftProperty.h"
#include "FaceliftLogging.h"


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

    facelift::Property<QString> m_property1;
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

    facelift::Property<QString> m_property2;
};
