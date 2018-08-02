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

function basic() {
    var idata;
    api.getIntValueAsync(function(val) {
            idata = val;
    });
    tryVerify(function() { return idata; }, 2000);
    compare(idata, 10);

    var sdata;
    api.getStringValueAsync(function (val) {
            sdata = val;
    });
    tryVerify(function() { return sdata; }, 2000);
    compare(sdata, "Test-String");

    var con;
    api.getContainerValueAsync(function(val) {
            con = val;
    });
    tryVerify(function() { return con; }, 2000);
    compare(con.integerData, 9);
    compare(con.stringData, "Test-String");
    compare(con.mapData["Test-String"], 27);

    var sum;
    api.calculateSumAsync(1, 2, 3, function(val) {
            sum = val;
    });
    tryVerify(function() { return sum; }, 2000);
    compare(sum, 6);

    var str;
    api.append("Deutsch", "Land", function(val) {
            str = val;
    });
    tryVerify(function() { return str; }, 2000);
    compare(str, "DeutschLand");
}
