#pragma once

#include "facelift/test/TestInterfacePropertyAdapter.h"
#include "facelift/test/TestInterface2PropertyAdapter.h"


using namespace facelift::test;

/**
 * C++ Implementation of the TestInterface API
 */
class TestInterfaceCppImplementation : public TestInterfacePropertyAdapter
{

    class Interface2Implementation : public TestInterface2PropertyAdapter {

    public:
        Interface2Implementation(TestInterfaceCppImplementation* parent, QString id) : TestInterface2PropertyAdapter(parent) {
            m_id = id;
        }

        void doSomething() override {
            qWarning() << "doSomething called. id =" << m_id;
        }

        QString m_id;
    };


public:
    TestInterfaceCppImplementation(QObject *parent = nullptr) : TestInterfacePropertyAdapter(parent)
    {
        qDebug() << "C++ implementation of TestInterface is used";
        m_readyProperty = 0;
        m_readyProperty.setReadiness(false);
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

        m_interfaceListProperty.addElement(new Interface2Implementation(this, ""));
        m_interfaceMapProperty = facelift::Map<facelift::test::TestInterface2*>({{"key1", new Interface2Implementation(this, "blabla")}});
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

    void setstructProperty2(const TestStruct2 & /*newValue*/) override
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
};
