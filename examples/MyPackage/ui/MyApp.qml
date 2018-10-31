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

//! [indoc]
import QtQuick 2.0
import facelift.example.mypackage 1.0

/**
 * This file contains the UI code of the application.
 */
Item {

    id: root

    // Instantiate our implementation and store it as a typed property 
    readonly property MyInterface myInterface: MyInterfaceImplementation {
        // Create an instance of the QML type registered under the name "MyInterfaceImplementation".
        // The exact type depends on what has been registered during the plugin initialization, but the interface implemented by any type 
        // registered under the name "MyInterface" should be the same. That enables the UI code to be reused whatever implementation is registered.
        onCounterReset: print("Counter has been reset")
    }

    MouseArea {
        anchors.fill: parent
        onClicked: myInterface.resetCounter(1000);
    }

    Text {
       text: myInterface.counter
    }

}
//! [indoc]
