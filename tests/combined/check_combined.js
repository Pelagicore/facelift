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

    compare(api.boolProperty, false);
    compare(api.enumProperty, CombiEnum.E1);
    compare(api.writableEnumProperty, 0);
    compare(api.intProperty, 0);

    compare(api.structProperty.anInt, 0);
    compare(api.structProperty.aString, "");
    compare(api.structProperty2.cs.anInt, 0);
    compare(api.structProperty2.cs.aString, "");
    compare(api.structProperty2.e, 0);

    compare(api.intListProperty, []);
    compare(api.boolListProperty.length, 0);
    compare(api.enumListProperty.length, 0);
    compare(api.stringListProperty.length, 0);
    compare(api.structListProperty.length, 0);

    compare(api.intMapProperty.one, undefined);
}

function initialized() {
    api.initialize();

    tryVerify(function() { return api.boolProperty; });

    compare(api.boolProperty, true);
    compare(api.enumProperty, CombiEnum.E2);
    compare(api.writableEnumProperty, 2);
    compare(api.intProperty, 17);

    compare(api.structProperty.anInt, 21);
    compare(api.structProperty.aString, "ok");
    compare(api.structProperty2.cs.anInt, 21);
    compare(api.structProperty2.cs.aString, "ok");
    compare(api.structProperty2.e, 1);

    if (!api.qmlImplementationUsed) {
        api.interfaceProperty.doSomething();
    }

    compare(api.intListProperty.length, 5);
    compare(api.intListProperty[2], 3);
    compare(api.boolListProperty.length, 3);
    compare(api.boolListProperty[2], true);
    compare(api.enumListProperty.length, 1);
    compare(api.enumListProperty[0], CombiEnum.E2);
    compare(api.stringListProperty.length, 3);
    compare(api.stringListProperty[0], "one");

    if (!api.qmlImplementationUsed) {
        compare(api.structListProperty.length, 2);
        compare(api.structListProperty[0].anInt, 21);
        compare(api.structListProperty[0].aString, "ok");
        compare(api.structListProperty[1].aString, "nok");
    }

    compare(api.enumMapProperty.one, CombiEnum.E1);
    compare(api.enumMapProperty.two, CombiEnum.E2);

    compare(api.intMapProperty.one, 1);
    compare(api.intMapProperty.two, 2);
}

function setter() {
    api.intProperty = -12;
    compare(api.intProperty, 0);

    api.stringListProperty = [ "11", "22"];
    compare(api.stringListProperty[0], "11");
}
