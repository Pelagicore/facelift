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

#include "tests/asynctests/AsyncTestsInterfacePropertyAdapter.h"


using namespace tests::asynctests;


class AsyncTestsInterfaceCppImplementation : public AsyncTestsInterfacePropertyAdapter
{
    public:
    
    AsyncTestsInterfaceCppImplementation(QObject *parent = nullptr) : AsyncTestsInterfacePropertyAdapter(parent)
    {
        qDebug() << "C++ implementation is used.";
    }

    
    void setvalue(const int& newValue) override
    {
        m_value= newValue;
    }

    void getIntValueAsync(facelift::AsyncAnswer<int> answer) override
    {
        qDebug()<<" C++ implementation setvalueasync is used";

        QTimer::singleShot(1000, [this, answer]() mutable {
            int returnValue = 10;
            answer(returnValue);
        });
    }
    
    void 
    getStringValueAsync(facelift::AsyncAnswer<QString> answer) override
    {
        qDebug()<<" C++ Implementation getStringValueAsync ";
        QTimer::singleShot(1000, [this, answer]() mutable {
            answer(QString("Test-String"));
        });
    }

    void 
    getContainerValueAsync(facelift::AsyncAnswer<Container> answer) override
    {
        qDebug()<<" C++ Implementation getStringValueAsync ";
        QTimer::singleShot(1000, [this, answer]() mutable {

                Container c;
                c.setidata(9);
                c.setsdata(QString("Test-String"));
                QMap<QString ,int> m;
                m["Test-String"]=27;
                answer(c);
        });
    }


    void calculateSumAsync(int arg1, int arg2, int arg3, facelift::AsyncAnswer<int> answer) override
    {
        QTimer::singleShot(1000, [this, answer,arg1,arg2,arg3]() mutable {

         answer(arg1 + arg2 + arg3);
        });
    }



//    void addEnumsAsync(QList<tests::asynctests::AsyncEnums> args, facelift::AsyncAnswer<tests::asynctests::AsyncEnums> answer) override
    //{
/*        QTimer::singleShot(1000, [this, answer,args]() mutable {
            int sum=0;
            for(auto it: args)
            {
                sum+it;
            }
            answer(sum);
        });*/
    //}


    
    
    void append(QString str1, QString str2, facelift::AsyncAnswer<QString> answer) override
    {
        QTimer::singleShot(1000,[this,answer,str1,str2]() mutable {
            answer(str1.append(str2));
        });
    }


    AsyncEnums compareEnums(tests::asynctests::AsyncEnums arg1,tests::asynctests::AsyncEnums arg2)
    {
        AsyncEnums e = AsyncEnums::EASYNC1;
        if( arg1==arg2)
            e =  AsyncEnums::EASYNC0;
        return e;
    }

  /*  bool compareEnums(tests::asynctests::AsyncEnums arg1,tests::asynctests::AsyncEnums arg2,facelift::AsyncAnswer<bool> answer) 
    {
        QTimer::singleShot(1000,[this,answer,str1,str2]() {
            answer(arg1==arg2);
        });
    }*/


  

    int calculateSum(int arg1,int arg2,int arg3) override
    {
        return arg1+arg2+arg3;
    }
};
