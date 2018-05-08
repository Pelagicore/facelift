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
import facelift.test 1.0

Rectangle {
    width: 400
    height: 300
    color: "green"

    TestInterfaceImplementation {
        intProperty: 5
        stringListProperty: [ 'one', 'two' ]
        intMapProperty: { 'one':1, 'two':2 }

        onEventWithList: console.log("Received signal with list parameter: " + p);
        onEventWithMap: console.log("Received signal with map parameter, e.g.: " + p.five);
        onEventWithStructWithList: console.log("Received signal with struct parameter. list of ints: "
                                               + p.listOfInts + " / list of structs: " + p.listOfStructs);
        onReadyFlagsChanged: console.log("ReadyFlag of readyProperty is " + readyFlags.readyProperty);

        Component.onCompleted: {
            console.log("Initial readyFlag of readyProperty: " + readyFlags.readyProperty);
            console.log("stringListProperty: " + stringListProperty);
            console.log("interfaceListProperty: " + interfaceListProperty[0]);
            console.log("intMapProperty: " + JSON.stringify(intMapProperty));
            interfaceListProperty[0].doSomething();
            console.log("intMapProperty.one: " + intMapProperty.one
                        + ", intMapProperty['two']: "+ intMapProperty['two']);

            console.log("interfaceMapProperty[\"key1\"]: " + interfaceMapProperty["key1"]);
            interfaceMapProperty["key1"].doSomething();
        }
    }
}
