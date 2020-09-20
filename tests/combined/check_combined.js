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

function defaults() {
    tryVerify(function() { return api.ready; });

    compare(api.isInitialized, false);
    compare(api.enumProperty, CombiEnum.E1);
    compare(api.writableEnumProperty, 0);
    compare(api.intProperty, 0);

    compare(api.structProperty.anInt, 0);
    compare(api.structProperty.aString, "");
    compare(api.structProperty2.cs.anInt, 0);
    compare(api.structProperty2.cs.aString, "");
    compare(api.structProperty2.e, 0);

    compare(api.structWithExtDependency.otherEnums, []);

    compare(api.intListProperty, []);
    compare(api.boolListProperty.length, 0);
    compare(api.enumListProperty.length, 0);
    compare(api.stringListProperty.length, 0);
    compare(api.structListProperty.length, 0);

    compare(api.intMapProperty.one, undefined);
}

function initialized() {
    api.initialize();
    tryVerify(function() { return api.isInitialized; });
    spy.otherDest = api.otherInterfaceProperty

    compare(api.enumProperty, CombiEnum.E2);
    compare(api.writableEnumProperty, 2);
    compare(api.intProperty, 17);

    compare(api.structProperty.anInt, 21);
    compare(api.structProperty.aString, "ok");
    compare(api.structProperty2.cs.anInt, 21);
    compare(api.structProperty2.cs.aString, "ok");
    compare(api.structProperty2.e, 1);

    compare(api.structWithExtDependency.otherEnums[0], 1);

    compare(api.intListProperty.length, 5);
    compare(api.intListProperty[2], 3);
    compare(api.boolListProperty.length, 3);
    compare(api.boolListProperty[2], true);
    compare(api.enumListProperty.length, 1);
    compare(api.enumListProperty[0], CombiEnum.E2);
    compare(api.stringListProperty.length, 3);
    compare(api.stringListProperty[0], "one");

    compare(api.structListProperty.length, 2);
    compare(api.structListProperty[0].anInt, 21);
    compare(api.structListProperty[0].aString, "ok");
    compare(api.structListProperty[1].aString, "nok");

    compare(api.enumMapProperty.one, CombiEnum.E1);
    compare(api.enumMapProperty.two, CombiEnum.E2);

    compare(api.intMapProperty.one, 1);
    compare(api.intMapProperty.two, 2);

    compare(api.emptyString, "");
}

function methods() {
    compare(api.method1(), "foo");

    var cs2 = api.method2(12, true);
    compare(cs2.cs.anInt, 13);
    compare(cs2.cs.aString, "bar");
    compare(cs2.e, CombiEnum.E2);

    compare(api.method3(CombiEnum.E2), CombiEnum.E3)

    var cs = CombiStructFactory.create();
    cs.aString = "hello";
    cs.anInt = 14;
    var cs25 = CombiStruct2Factory.create();
    cs25.cs = cs;
    cs25.e = CombiEnum.E2;
    var lce = api.method4(cs25);
    compare(lce[0], CombiEnum.E3);
    compare(lce[1], CombiEnum.E1);

    var lcs = api.method5();
    compare(lcs[0].anInt, 1);
    compare(lcs[0].aString, "A");
    compare(lcs[1].anInt, 2);
    compare(lcs[1].aString, "B");

    compare(api.method6(17), 42)

    var os = OtherStructFactory.create();
    os.ival = 101;
    compare(api.method7(os), OtherEnum.O3);

    compare(api.listOfAnotherEnums()[0], OtherEnum.O2);

    if (!api.qmlImplementationUsed) {
        api.interfaceProperty.doSomething();
        compare(api.otherInterfaceProperty.otherMethod(OtherEnum.O3), "O3");

        var asyncResult = { answer: 0 };
        api.otherInterfaceProperty.asyncFunction(function(result) {
            asyncResult.answer = result;
        });
        tryCompare(asyncResult, "answer", 42);

        api.interfaceMapProperty["key1"].doSomething();
        api.interfaceListProperty[0].doSomething();
    }
}

function methodsAsync() {
    var retval;

    var callback = function(value) {
        retval = value;
    }

    var getretval = function() {
        return retval;
    }

    api.method1(callback);
    tryVerify(getretval);
    compare(retval, "foo");

    retval = undefined;
    api.method2(12, true, callback);
    compare(retval, undefined);
    tryVerify(getretval);
    compare(retval.cs.anInt, 13);
    compare(retval.cs.aString, "bar");
    compare(retval.e, CombiEnum.E2);

    retval = undefined;
    api.method3(CombiEnum.E2, callback);
    compare(retval, undefined);
    tryVerify(getretval);
    compare(retval, CombiEnum.E3);

    var cs = CombiStructFactory.create();
    cs.aString = "hello";
    cs.anInt = 14;
    var cs25 = CombiStruct2Factory.create();
    cs25.cs = cs;
    cs25.e = CombiEnum.E2;
    retval = undefined;
    api.method4(cs25, callback);
    compare(retval, undefined);
    tryVerify(getretval);
    compare(retval[0], CombiEnum.E3);
    compare(retval[1], CombiEnum.E1);

    retval = undefined;
    api.method5(callback);
    compare(retval, undefined);
    tryVerify(getretval);
    compare(retval[0].anInt, 1);
    compare(retval[0].aString, "A");
    compare(retval[1].anInt, 2);
    compare(retval[1].aString, "B");

    retval = undefined;
    api.method6(17, callback);
    compare(retval, undefined);
    tryVerify(getretval);
    compare(retval, 42)
}

function setter() {
    spy.intPropertyChangedSpy.clear();
    api.intProperty = -12;
    spy.intPropertyChangedSpy.wait(2000);  // needed in IPC case only
    compare(api.intProperty, 0);

    spy.stringListPropertyChangedSpy.clear();
    api.stringListProperty = [ "11", "22"];
    spy.stringListPropertyChangedSpy.wait(2000);  // needed in IPC case only
    compare(api.stringListProperty[0], "11");
}

function signals() {
    compare(spy.event1Spy.count, 0);
    compare(spy.eventCombiEnumSpy.count, 0);
    compare(spy.eventIntSpy.count, 0);
    compare(spy.eventBoolAndCombiStructSpy.count, 0);
    compare(spy.eventWithListSpy.count, 0);
    compare(spy.eventWithMapSpy.count, 0);
    compare(spy.eventWithStructWithListSpy.count, 0);
    compare(spy.otherEventSpy.count, 0);
    spy.intPropertyChangedSpy.clear();

    api.emitSignals();

    spy.event1Spy.wait(2000);
    compare(spy.event1Spy.count, 1);
    compare(spy.event1Spy.signalArguments[0][0].anInt, 21);
    compare(spy.event1Spy.signalArguments[0][0].aString, "ok");

    spy.eventCombiEnumSpy.wait(2000);
    compare(spy.eventCombiEnumSpy.count, 1);
    compare(spy.eventCombiEnumSpy.signalArguments[0][0], CombiEnum.E2);

    spy.eventIntSpy.wait(2000);
    compare(spy.eventIntSpy.count, 1);
    compare(spy.eventIntSpy.signalArguments[0][0], 7);

    spy.eventBoolAndCombiStructSpy.wait(2000);
    compare(spy.eventBoolAndCombiStructSpy.count, 1);
    compare(spy.eventBoolAndCombiStructSpy.signalArguments[0][0], true);
    compare(spy.eventBoolAndCombiStructSpy.signalArguments[0][1].anInt, 21);
    compare(spy.eventBoolAndCombiStructSpy.signalArguments[0][1].aString, "ok");

    spy.eventWithListSpy.wait(2000);
    compare(spy.eventWithListSpy.count, 1);
    compare(spy.eventWithListSpy.signalArguments[0][0][0], 1);
    compare(spy.eventWithListSpy.signalArguments[0][0][4], 8);
    compare(spy.eventWithListSpy.signalArguments[0][1], true);

    spy.eventWithMapSpy.wait(2000);
    compare(spy.eventWithMapSpy.count, 1);
    compare(spy.eventWithMapSpy.signalArguments[0][0].one, 1);
    compare(spy.eventWithMapSpy.signalArguments[0][0].two, 2);

    spy.eventWithStructWithListSpy.wait(2000);
    compare(spy.eventWithStructWithListSpy.count, 1);
    compare(spy.eventWithStructWithListSpy.signalArguments[0][0].listOfInts[3], 5);
    compare(spy.eventWithStructWithListSpy.signalArguments[0][0].listOfInts[4], 8);
    compare(spy.eventWithStructWithListSpy.signalArguments[0][0].listOfStructs[0].anInt, 21);
    compare(spy.eventWithStructWithListSpy.signalArguments[0][0].listOfStructs[0].aString, "ok");
    compare(spy.eventWithStructWithListSpy.signalArguments[0][0].enumField, CombiEnum.E2);

    if (!api.qmlImplementationUsed) {   // "sub" interfaces not supported via QML
        spy.otherEventSpy.wait(2000);
        compare(spy.otherEventSpy.count, 1);
        compare(spy.otherEventSpy.signalArguments[0][0].ival, 12);
    }

    spy.intPropertyChangedSpy.wait(2000);
    compare(spy.intPropertyChangedSpy.count, 1);
    compare(api.intProperty, 101);
}
