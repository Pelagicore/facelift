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

#include "test1.h"

#include <QDebug>


class MyAddressBookImplementation : public AddressBookImplementation
{

    QString property1() const override
    {
        return "gggg";
    }

    void do1() override
    {
    }

};


class MyAddressBookImplementationWithProperties : public AddressBookImplementationWithProperties
{

public:
    void do1() override
    {
        signal1();
    }

};


class MyAddressBookExtendedImplementation : public MyAddressBookImplementationWithProperties
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
