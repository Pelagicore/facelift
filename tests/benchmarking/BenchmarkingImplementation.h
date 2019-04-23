/**********************************************************************
**
** Copyright (C) 2019 Luxoft Sweden AB
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

#include "facelift/tests/benchmarking/BenchmarkingImplementationBase.h"

using namespace facelift::tests::benchmarking;

class BenchmarkingImplementation : public  BenchmarkingImplementationBase
{
public:
    BenchmarkingImplementation()
    {
        m_smallModel.reset(2000, std::bind(&BenchmarkingImplementation::smallModelData, this, std::placeholders::_1));
        m_largeModel.reset(2000, std::bind(&BenchmarkingImplementation::largelModelData, this, std::placeholders::_1));

        m_someString = "test";
        m_someInteger = 10;
        m_someEnum = BenchmarkingEnum::BE1;

        LargeStruct testStr;
        testStr.setid(42);
        testStr.setname("test");
        testStr.setsomeEnum(BenchmarkingEnum::BE1);
        m_someStruct.setValue(testStr);
    }

    SmallStruct smallModelData(int)
    {
        SmallStruct entry;
        entry.setid(42);
        return entry;
    }

    LargeStruct largelModelData(int )
    {
        LargeStruct entry;
        entry.setid(42);
        entry.setname(QStringLiteral("The quick brown fox jumps over the lazy dog. "
                                     "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG. "
                                     "The quick brown fox jumps over the lazy dog. "
                                     "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG."));

        entry.setsomeEnum(BenchmarkingEnum::BE1);
        return entry;
    }

    void requestInteger(int intData, facelift::AsyncAnswer<int> answer) override
    {
        QTimer::singleShot(0, [answer, intData]() mutable {
            answer(intData);
        });
    }

    void requestString(const QString &someString, facelift::AsyncAnswer<QString> answer) override
    {
        QTimer::singleShot(0, [answer, someString]() mutable {
            answer(someString);
        });
    }

    void requestStructure(const LargeStruct &structData, facelift::AsyncAnswer<LargeStruct> answer) override
    {
        QTimer::singleShot(0, [answer, structData]() mutable {
            answer(structData);
        });
    }
};
