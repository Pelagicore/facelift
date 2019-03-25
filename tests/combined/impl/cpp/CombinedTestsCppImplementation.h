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

#include "tests/combined/CombinedInterfaceImplementationBase.h"
#include "tests/combined/CombinedInterface2ImplementationBase.h"
#include "tests/combined/other/OtherInterfaceImplementationBase.h"
#include "FaceliftLogging.h"

using namespace tests::combined;
using namespace tests::combined::other;


class OtherInterfaceImplementation : public OtherInterfaceImplementationBase
{
public:
    OtherInterfaceImplementation(QObject *parent = nullptr) : OtherInterfaceImplementationBase(parent) {}

    QString otherMethod(OtherEnum oe) override
    {
        if (oe == OtherEnum::O3)
            return QStringLiteral("O3");
        return QString();
    }
};


class CombinedInterfaceImplementation : public CombinedInterfaceImplementationBase
{
    class CombinedInterface2Implementation : public CombinedInterface2ImplementationBase
    {
    public:
        CombinedInterface2Implementation(CombinedInterfaceImplementation* parent, QString id)
            : CombinedInterface2ImplementationBase(parent)
        {
            m_id = id;
        }

        void doSomething() override
        {
            qCDebug(LogGeneral) << "doSomething() called, id =" << m_id;
        }

        QString m_id;
    };

public:
    CombinedInterfaceImplementation(QObject *parent = nullptr) : CombinedInterfaceImplementationBase(parent)
    {
        qCDebug(LogGeneral) << "C++ implementation is used.";
        m_readyProperty = 0;
        m_readyProperty.setReady(false);
    }

    void initialize() override
    {
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
        m_otherInterfaceProperty.setValue(new OtherInterfaceImplementation(this));

        m_intListProperty = { 1, 2, 3, 5, 8 };
        m_boolListProperty = { false, true, true };
        m_enumListProperty = { CombiEnum::E2 };
        m_stringListProperty = { QStringLiteral("one"), QStringLiteral("two"), QStringLiteral("three") };

        CombiStruct c;
        c.setaString("nok");
        m_structListProperty = { s, c };

        m_enumMapProperty = QMap<QString, CombiEnum> { { "one", CombiEnum::E1 }, { "two", CombiEnum::E2 } };
        m_intMapProperty = QMap<QString, int> { { "one", 1 }, { "two", 2 } };

        m_readyProperty = 42;
        m_isInitialized = true;
    }

    void emitSignals() override
    {
        emit event1(m_structProperty);
        emit eventCombiEnum(CombiEnum::E2);
        emit eventInt(7);
        emit eventBoolAndCombiStruct(true, m_structProperty);
        emit eventWithList(m_intListProperty, true);
        emit eventWithMap(m_intMapProperty);

        StructWithList swl;
        swl.setlistOfInts(m_intListProperty);
        swl.setlistOfStructs(m_structListProperty);
        swl.setenumField(CombiEnum::E2);
        emit eventWithStructWithList(swl);

        OtherStruct os;
        os.setival(12);
        emit m_otherInterfaceProperty.value()->otherEvent(os);

        m_intProperty = 101;
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
        return QStringLiteral("foo");
    }

    CombiStruct2 method2(int intParam, bool boolParam) override
    {
        if (intParam == 12 && boolParam) {
            CombiStruct cs;
            cs.setanInt(++intParam);
            cs.setaString("bar");
            CombiStruct2 cs2;
            cs2.setcs(cs);
            cs2.sete(CombiEnum::E2);
            return cs2;
        } else {
            return CombiStruct2();
        }
    }

    CombiEnum method3(CombiEnum e) override
    {
        if (e == CombiEnum::E2)
            return CombiEnum::E3;
        return CombiEnum::E1;
    }

    QList<CombiEnum> method4(CombiStruct2 s)  override
    {
        if (s.cs().anInt() == 14 && s.cs().aString() == "hello" && s.e() == CombiEnum::E2) {
            QList<CombiEnum> lce = { CombiEnum::E3, CombiEnum::E1 };
            return lce;
        }
        return QList<CombiEnum>();
    }

    QList<CombiStruct> method5() override
    {
        CombiStruct c1;
        c1.setanInt(1);
        c1.setaString("A");

        CombiStruct c2;
        c2.setanInt(2);
        c2.setaString("B");

        QList<CombiStruct> lcs = { c1, c2 };

        return lcs;
    }

    int method6(int i) override
    {
        if (i == 17)
            return 42;
        return 0;
    }

    OtherEnum method7(OtherStruct os) override
    {
        if (os.ival() == 101)
            return OtherEnum::O3;
        return OtherEnum::O1;
    }
};
