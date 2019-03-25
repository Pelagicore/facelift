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

#include "tests/userData/ProducerImplementationBase.h"
#include "tests/userData/ConsumerImplementationBase.h"
#include "FaceliftLogging.h"


using namespace tests::userData;


struct Pod
{
    int i;
    bool b;
};

Q_DECLARE_METATYPE(Pod)


class ProducerImplementation : public ProducerImplementationBase
{
public:
    ProducerImplementation(QObject *parent = nullptr) : ProducerImplementationBase(parent) {}

    void produceUserData(Type type) override
    {
        TestStruct ts;
        ts.seti(21);
        ts.setstr("ok");

        switch(type) {
        case Type::E_INT:
            ts.setId(1);
            ts.setUserData(42);
            break;
        case Type::E_STRING:
            ts.setId(2);
            ts.setUserData(QStringLiteral("foo"));
            break;
        case Type::E_STRUCT:
            ts.setId(3);
            {
                Pod pod = { 3, true };
                ts.setUserData(pod);
            }
            break;
        case Type::E_MAP:
            ts.setId(4);
            {
                QVariantMap vm;
                vm["one"] = QVariant::fromValue(42);
                vm["two"] = QVariant(QStringLiteral("foo"));
                Pod pod = { 4, true };
                vm["three"] = QVariant::fromValue(pod);
                ts.setUserData(vm);
            }
            break;
        }

        m_testStruct = ts;
    }
};


class ConsumerImplementation : public ConsumerImplementationBase
{
public:
    ConsumerImplementation(QObject *parent = nullptr) : ConsumerImplementationBase(parent) {}

    bool consumeUserData(Type type, TestStruct ts) override
    {
        bool res = false;
        switch(type) {
        case Type::E_INT:
            if (ts.id() == 1 && ts.userData<int>() == 42)
                res = true;
            break;
        case Type::E_STRING:
            if (ts.id() == 2 && ts.userData<QString>() == QLatin1String("foo"))
                res = true;
            break;
        case Type::E_STRUCT:
            if (ts.id() == 3 && ts.userData<Pod>().i == 3 && ts.userData<Pod>().b)
                res = true;
            break;
        case Type::E_MAP:
            QVariantMap vm = ts.userData<QVariantMap>();
            if (ts.id() == 4 && vm.value("one") == 42 && vm.value("two") == QLatin1String("foo")
                                                      && vm.value("three").value<Pod>().i == 4) {
                res = true;
            }
            break;
        }

        if (!res)
            qCInfo(LogGeneral) << "consumed:" << ts.id() << ts.userData();

        return res;
    }
};
