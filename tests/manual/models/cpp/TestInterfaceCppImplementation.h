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

#include <QTimer>

#include "facelift/test/TestInterfaceImplementationBase.h"
#include "facelift/test/TestInterface2ImplementationBase.h"
#include "FaceliftLogging.h"


using namespace facelift::test;

/**
 * C++ Implementation of the TestInterface API
 */
class TestInterfaceImplementation : public TestInterfaceImplementationBase
{

    class Interface2Implementation : public TestInterface2ImplementationBase {

    public:
        Interface2Implementation(TestInterfaceImplementation& main, QString id) : TestInterface2ImplementationBase(&main), m_main(main) {
            m_id = id;
        }

        void doSomething() override {
            qCWarning(LogGeneral) << "doSomething called. id =" << m_id;
        }

        void triggerMainInterfaceSignal(int signalParameter) override {
            m_main.aSignal(signalParameter);
        }

    private:
        QString m_id;
        TestInterfaceImplementation& m_main;
    };


public:
    TestInterfaceImplementation(QObject *parent = nullptr) : TestInterfaceImplementationBase(parent)
    {
        qDebug() << "C++ implementation of TestInterface is used";
        m_readyProperty = 0;
        m_readyProperty.setReady(false);
        QTimer::singleShot(1000, this, [this] () {
            m_readyProperty = 42;
            eventWithList(QList<int>({3, 4, 5}), true);
            eventWithMap(QMap<QString, int>({{QStringLiteral("five"), 5}, {QStringLiteral("six"), 6}}));

            QList<TestStruct> structs;
            TestStruct s;
            s.setaString("Struct1");
            structs.append(s);
            s.setaString("Struct2");
            structs.append(s);
            TestStructWithList arg;
            arg.setlistOfStructs(structs);
            arg.setlistOfInts(QList<int>({2, 5, 8}));
            eventWithStructWithList(arg);
        });

        m_interfaceListProperty.addElement(new Interface2Implementation(*this, ""));
        m_interfaceMapProperty = facelift::Map<facelift::test::TestInterface2*>({{"key1", new Interface2Implementation(*this, "blabla")}});
        m_interfaceProperty = new Interface2Implementation(*this, "");
    }

    void setintProperty(const int &newValue) override
    {
        qDebug() << "set intProperty:" << newValue;
        m_intProperty = newValue;
    }

    void setintMapProperty(const QMap<QString, int>& newValue) override
    {
        qDebug() << "set intMap:" << newValue;
        m_intMapProperty = newValue;
    }

    void setwritableEnumProperty(const TestEnum & /*newValue*/) override
    {
    }

    void setstructProperty(const TestStruct & /*newValue*/) override
    {
    }

    void asyncMethod(int arg, facelift::AsyncAnswer<int> answer) override
    {
        QTimer::singleShot(1000, [answer, arg]() mutable {
            answer(arg + 10);
        });
    }

    void setstructProperty2(const TestStruct2 & /*newValue*/) override;

    void setstringListProperty(const QList<QString> & newValue) override
    {
        qCDebug(LogGeneral) << "set stringListProperty:" << newValue;
        m_stringListProperty = newValue;
    }

    void setreadyProperty(const int &newValue) override
    {
        qCDebug(LogGeneral) << "set readyProperty:" << newValue;
        m_intProperty = newValue;
    }

    QString method1() override
    {
        return QString();
    }

    TestStruct2 method2(int /*intParam*/, bool /*boolParam*/) override
    {
        return TestStruct2();
    }

    TestEnum method3() override
    {
        return TestEnum();
    }

    QList<TestEnum> method4(TestStruct2 /*s*/)  override
    {
        return QList<TestEnum>();
    }

    QList<TestStruct> method5() override
    {
        return QList<TestStruct>();
    }

    TestInterface2 *methodWithSubInterface() override
    {
        return m_interfaceProperty;
    }

    void asyncMethodWithSubInterface(facelift::AsyncAnswer<TestInterface2 *> answer) override
    {
        answer(m_interfaceProperty);
    }

};
