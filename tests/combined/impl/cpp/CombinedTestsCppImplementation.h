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

#include "tests/combined/CombinedInterfacePropertyAdapter.h"
#include "tests/combined/CombinedInterface2PropertyAdapter.h"


using namespace tests::combined;


class CombinedInterfaceCppImplementation : public CombinedInterfacePropertyAdapter
{
    class CombinedInterface2Implementation : public CombinedInterface2PropertyAdapter
    {
    public:
        CombinedInterface2Implementation(CombinedInterfaceCppImplementation* parent, QString id)
            : CombinedInterface2PropertyAdapter(parent)
        {
            m_id = id;
        }

        void doSomething() override
        {
            qDebug() << "doSomething() called, id =" << m_id;
        }

        QString m_id;
    };

public:
    CombinedInterfaceCppImplementation(QObject *parent = nullptr) : CombinedInterfacePropertyAdapter(parent)
    {
        qDebug() << "C++ implementation is used.";
        m_readyProperty = 0;
        m_readyProperty.setReady(false);
    }

    void initialize() override
    {
        m_boolProperty = true;
        m_enumProperty = CombiEnum::E2;
        m_writableEnumProperty = CombiEnum::E3;
        m_intProperty = 17;
        CombiStruct s;
        s.setanInt(21);
        s.setaString("ok");
        m_structProperty = s;
        CombiStruct2 s2;
        s2.setcs(s);
        s2.sete(CombiEnum::E2);
        m_structProperty2 = s2;

        m_interfaceProperty.setValue(new CombinedInterface2Implementation(this, "#7"));

        m_intListProperty = { 1, 2, 3, 5, 8 };
        m_boolListProperty = { false, true, true };
        m_enumListProperty = { CombiEnum::E2 };
        m_stringListProperty = { QStringLiteral("one"), QStringLiteral("two"), QStringLiteral("three") };

        CombiStruct c;
        c.setaString("nok");
        m_structListProperty = { s, c };

        m_intMapProperty = QMap<QString, int> { { "one", 1 }, { "two", 2 } };

        m_readyProperty = 42;
    }

    void setintProperty(const int &newValue) override
    {
        m_intProperty = newValue > 0 ? newValue : 0;
    }

    void setintMapProperty(const QMap<QString, int>& newValue) override
    {
        m_intMapProperty = newValue;
    }

    void setwritableEnumProperty(const CombiEnum & /*newValue*/) override
    {
    }

    void setstructProperty(const CombiStruct & /*newValue*/) override
    {
    }

    void setstructProperty2(const CombiStruct2 & /*newValue*/) override
    {
    }

    void setstringListProperty(const QList<QString> & newValue) override
    {
        qDebug() << "set stringListProperty:" << newValue;
        m_stringListProperty = newValue;
    }

    void setreadyProperty(const int &newValue) override
    {
        qDebug() << "set readyProperty:" << newValue;
        m_intProperty = newValue;
    }

    QString method1() override
    {
        return QString();
    }

    CombiStruct2 method2(int /*intParam*/, bool /*boolParam*/) override
    {
        return CombiStruct2();
    }

    CombiEnum method3() override
    {
        return CombiEnum();
    }

    QList<CombiEnum> method4(CombiStruct2 /*s*/)  override
    {
        return QList<CombiEnum>();
    }

    QList<CombiStruct> method5() override
    {
        return QList<CombiStruct>();
    }
};
