
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
** SPDX-License-identifier: MIT
**
**********************************************************************/

function checkInit() {

    tryVerify(function() { return api.ready; });
    verify(api.smallModel);
    compare(api.smallModel.rowCount(), 2000);
    verify(api.largeModel);
    compare(api.largeModel.rowCount(), 2000);

    // fill cache
    compare(api.largeModel.data(api.largeModel.index(999, 0)).id, 42);
    compare(api.smallModel.data(api.smallModel.index(999, 0)).id, 42);

    // fill struct
    struct  =  api.LargeStructFactory.create();
    struct.id =  42
    struct.name = "test"
    struct.someEnum = api.someEnum.BE1
}


function readItemFromModel(model) {
    for(var i = 0; i < model.rowCount(); ++i) {
        compare(model.data(model.index(999, 0)).id, 42);
    }
}

function readItemsFromModelRandomly(model) {
    var random = Math.floor((Math.random() * 100) + 1), j = 0;
    for(var i = 0; i < model.rowCount(); ++i) {
        compare(model.data(model.index(j, 0)).id, 42);
        j = (j + random) % model.rowCount();
    }
}

function readItemsFromModelSequentially(model) {
    for(var i = 0; i < model.rowCount(); ++i) {
        compare(model.data(model.index(i, 0)).id, 42);
    }
}

function getInteger(api) {
    compare(api.someInteger,10);
}

function getString(api) {
    compare(api.someString, "test")
}

function getEnum(api) {
    compare(api.someEnum.BE1, api.someEnum.BE1);
}

function getStruct(api ) {
    compare(api.someStruct, struct)
}

function requestInteger(api) {
    var data;
    api.requestInteger(10, function(arg) {
        data = arg;
    });

    tryVerify(function() { return data; }, 5000);
    compare(10, data);
}

function requestString(api) {
    var data;
    api.requestString("test", function(arg) {
        data = arg;
    });

    tryVerify(function() { return data; }, 5000);
    compare("test", data);
}

function requestStruct(api) {
    var data;
    api.requestStructure(struct, function(arg) {
        data = arg
    });

    tryVerify(function() { return data; }, 5000);

    compare(struct, data);
}
