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

import QtQuick 2.0
import tests.combined 1.0


CombinedInterfaceImplementationBase {
    id: root
    qmlImplementationUsed: true

//    CombinedInterface2ImplementationBase {
//        id: ci2

//        doSomething: function() {
//            console.log("doSomething() called, id = " + id);
//        }
//    }

    setintProperty: function(i) {
        root.intProperty = i > 0 ? i : 0;
    }

    setstringListProperty: function(sl) {
        stringListProperty = sl;
    }

    method1: function() {
        return "foo";
    }

    method2: function(intParam, boolParam) {
        if (intParam === 12 && boolParam) {
            var cs = CombiStructFactory.create();
            cs.aString = "bar";
            cs.anInt = ++intParam;
            var cs2 = CombiStruct2Factory.create();
            cs2.cs = cs;
            cs2.e = CombiEnum.E2;
            return cs2;
         }
    }

    method3: function(e) {
        if (e === CombiEnum.E2)
            return CombiEnum.E3;
    }

    method4: function(s) {
        if (s.cs.anInt === 14 && s.cs.aString === "hello" && s.e === CombiEnum.E2) {
            return [ CombiEnum.E3, CombiEnum.E1];
        }
    }

    method5: function() {
        var c1 = CombiStructFactory.create();
        c1.anInt = 1;
        c1.aString = "A";
        var c2 = CombiStructFactory.create();
        c2.anInt = 2;
        c2.aString = "B";
        return [ c1, c2 ];
    }

    method6: function(i) {
        if (i === 17)
            return 42;
    }

    method7: function(os) {
        if (os.ival === 101)
            return OtherEnum.O3;
        return OtherEnum.O1;
    }

    listOfAnotherEnums: function() {
        return [OtherEnum.O2];
    }


    initialize: function() {
        enumProperty = CombiEnum.E2;
        writableEnumProperty = CombiEnum.E3;
        intProperty = 17;

        structProperty.anInt = 21;
        structProperty.aString = "ok";
        structProperty2.cs = structProperty;
        structProperty2.cs.aString = "ok";
        structProperty2.e = CombiEnum.E2;

        structWithExtDependency.otherEnums = [OtherEnum.O2];
        structWithExtDependency.otherEnumMap = {"O1": OtherEnum.O1, "O2": OtherEnum.O2};

        intListProperty = [ 1, 2, 3, 5, 8 ];
        boolListProperty = [ false, true, true ];
        enumListProperty = [ CombiEnum.E2 ];
        stringListProperty = [ "one", "two", "three" ];

        var cs1 = CombiStructFactory.create();
        cs1.aString = "ok";
        cs1.anInt = 21;
        var cs2 = CombiStructFactory.create();
        cs2.aString = "nok";
        structListProperty = [ cs1, cs2 ]

        enumMapProperty = { one: CombiEnum.E1, two: CombiEnum.E2 };
        intMapProperty = { one: 1, two: 2 };
        isInitialized = true;
    }

    emitSignals: function() {
        event1(structProperty);
        eventCombiEnum(CombiEnum.E2);
        eventInt(7);
        eventBoolAndCombiStruct(true, structProperty);
        eventWithList(intListProperty, true);
        eventWithMap(intMapProperty);

        var swl = StructWithListFactory.create();
        swl.listOfInts = intListProperty;
        swl.listOfStructs = structListProperty;
        swl.enumField = CombiEnum.E2;
        eventWithStructWithList(swl);
        intProperty = 101;
    }

    Component.onCompleted: console.log("QML implementation is used.");
}
