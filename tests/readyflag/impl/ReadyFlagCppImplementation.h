#pragma once

#include "tests/readyflag/ReadyFlagInterfacePropertyAdapter.h"

using namespace tests::readyflag;

class ReadyFlagInterfaceCppImplementation : public ReadyFlagInterfacePropertyAdapter
{
public:

    ReadyFlagInterfaceCppImplementation(QObject *parent = nullptr) : ReadyFlagInterfacePropertyAdapter(parent)
    {
        m_intProperty = 0;
        m_intProperty.setReady(false);        

        m_strProperty.setReady(false);

        m_comboData.setReady(false);
    }

    void setintProperty(const int& newValue) override {
        m_intProperty =  newValue;
    }

    void setstrProperty(const QString& newValue) override {
        m_strProperty = newValue;
    }

    void setcomboData(const tests::readyflag::_comboData& newValue) override {
        m_comboData = newValue;
    }
};

