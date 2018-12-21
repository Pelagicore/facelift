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

import QtQuick 2.6
import QtTest 1.2
import tests.combined 1.0
import "check_combined.js" as Check


TestCase {
    name: "combined-inprocess"

    Loader {
        id: implLoader
        sourceComponent: CombinedInterfaceAPI {
            IPC.enabled: true
            IPC.objectPath: "/tests/combined/inprocess"
        }
    }

    Loader {
        id: apiLoader
        sourceComponent: CombinedInterfaceIPCProxy {
            ipc.objectPath: "/tests/combined/inprocess"
        }
    }

    property alias api: apiLoader.item

    CombinedSignalSpys {
        id: spy
        dest: api
    }


    function initTestCase() {
        Check.defaults();
        Check.initialized();
    }

    function test_methods() {
        Check.methods();
    }

    function test_setter() {
        Check.setter();
    }

    function test_signals() {
        Check.signals();
    }

    function test_testcrash() {
        apiLoader.active = false;
        implLoader.active = false;
        wait(1000);
        implLoader.active = true;
        apiLoader.active = true;
    }
}
