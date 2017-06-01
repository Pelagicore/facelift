/*
 *   This is part of the QMLCppAPI project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 *   Please see the LICENSE file for details.
 */

#include "test1.h"

#include <QDebug>


class MyAddressBookImplementation :
    public AddressBookImplementation
{

    QString property1() const
    {
        return "gggg";
    }

    void do1()
    {
    }

};


class MyAddressBookImplementationWithProperties :
    public AddressBookImplementationWithProperties
{

public:
    void do1()
    {
        signal1();
    }

};


class MyAddressBookExtendedImplementation :
    public MyAddressBookImplementationWithProperties
{

public:
    void do2()
    {
        //        signal2();
    }

};


int main()
{

    MyAddressBookImplementation i1;
    MyAddressBookImplementationWithProperties i2;
    MyAddressBookExtendedImplementation i3;

    i3.property1();
    //    i3.property2();
    i3.do1();
    i3.do2();

    qDebug() << "OKKK";
    return 0;
}
