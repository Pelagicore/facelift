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

import QtTest 1.2
import tests.readyflag  1.0

TestCase {
    ReadyFlagInterfaceAPI {
        id: api
    }

    function initTestCase() {
        verify(!api.readyFlags.intProperty);
        api.intProperty = 10;
        verify(api.readyFlags.intProperty);
        compare(api.intProperty, 10);

        verify(!api.readyFlags.strProperty)
        api.strProperty = "hello";
        verify(api.readyFlags.strProperty);
        compare(api.strProperty, "hello");

        verify(!api.readyFlags.comboData);
        api.comboData.iData = 100;
        verify(api.readyFlags.comboData);
        api.comboData.sData = "test";
        api.comboData.bData = true;
        verify(api.readyFlags.comboData);
        compare(api.comboData.iData, 100);
        compare(api.comboData.sData, "test");
        compare(api.comboData.bData, true);
    }

    function test_reset() {
        verify(api.readyFlags.intProperty);
        verify(api.readyFlags.comboData);
        api.reset();
        verify(!api.readyFlags.intProperty);
        verify(!api.readyFlags.comboData);
        api.comboData.iData = 1;
        verify(api.readyFlags.comboData);
        tryVerify(function() { return api.readyFlags.intProperty; });
        compare(api.intProperty, 42);
    }
}
