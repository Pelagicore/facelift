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
** SPDX-License-Identifier: MIT
**
**********************************************************************/

import QtTest 1.2
import tests.benchmarking 1.0
import "BenchmarkingHelper.js" as JsBenchmarking

TestCase {
    name: "benchmark-models-inprocess"
    property  var struct;

    Benchmarking {
        IPC.enabled: true
    }

    BenchmarkingIPCProxy {
        id: api
    }

    function initTestCase() {
        JsBenchmarking.checkInit();
    }

    function benchmark_readItemFromLargeModelOnce() {
        JsBenchmarking.readItemFromModel(api.largeModel);
    }

    function benchmark_readItemsFromLargeModelRandomly() {
        JsBenchmarking.readItemsFromModelRandomly(api.largeModel);
    }

    function benchmark_readItemsFromLargeModelSequentially() {
        JsBenchmarking.readItemsFromModelSequentially(api.largeModel);
    }

    function benchmark_readItemFromSmallModelOnce() {
        JsBenchmarking.readItemFromModel(api.largeModel);
    }

    function benchmark_readItemsFromSmallModelRandomly() {
        JsBenchmarking.readItemsFromModelRandomly(api.largeModel);
    }

    function benchmark_readItemsFromSmallModelSequentially() {
        JsBenchmarking.readItemsFromModelSequentially(api.largeModel);
    }

    //sync

    function benchmark_getEnumSync()
    {
        JsBenchmarking.getEnum(api);
    }

    function benchmark_getIntegerSync()
    {
        JsBenchmarking.getInteger(api);
    }

    function benchmark_getStringSync()
    {
        JsBenchmarking.getString(api);
    }

    function benchmark_getStructSync()
    {
        JsBenchmarking.getStruct(api);
    }

    //async

    function benchmark_requestIntegerASync()
    {
        JsBenchmarking.requestInteger(api);
    }

    function benchmark_requestStringASync()
    {
        JsBenchmarking.requestString(api);
    }

    function benchmark_requestStructASync()
    {
        JsBenchmarking.requestStruct(api);
    }
}
