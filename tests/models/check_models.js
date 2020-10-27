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


function index(i) {
    return api.theModel.index(i, 0);
}

function getData(i) {
    return api.theModel.data(index(i));
}


function checkInit()
{
    tryVerify(function() { return api.ready; });
    verify(api.theModel);
    compare(api.theModel.rowCount(), 100);
    compare(api.oversizedStruct.theKiller.anotherSecondLevel.firstLevel.someOtherString, "someOtherString");
    for(var i = 0; i < api.theModel.rowCount(); ++i) {
        compare(getData(i).name, "entry " + i);
        compare(getData(i).enabled, !(i%2));
    }
}

function checkModelChanges()
{
    dataChangedSpy.clear();
    api.renameModelItem(8, 8, 4201);
    dataChangedSpy.wait(1000);
    compare(dataChangedSpy.count, 1);
    compare(dataChangedSpy.signalArguments[0][0], index(8));
    compare(dataChangedSpy.signalArguments[0][1], index(8));
    compare(getData(7).name, "entry 7");
    compare(getData(8).name, "entry 4201");
    compare(getData(9).name, "entry 9");

    dataChangedSpy.clear();
    api.renameModelItem(20, 24, 4202);
    dataChangedSpy.wait(1000);
    compare(dataChangedSpy.count, 1);
    compare(dataChangedSpy.signalArguments[0][0], index(20));
    compare(dataChangedSpy.signalArguments[0][1], index(24));
    compare(getData(19).name, "entry 19");
    for(var i = 20; i < 24; ++i) {
        compare(getData(i).name, "entry 4202");
    }
    compare(getData(25).name, "entry 25");

    rowsAboutToBeInsertedSpy.clear();
    rowsInsertedSpy.clear();
    api.insertNewModelItems(19, 19);
    rowsAboutToBeInsertedSpy.wait(1000);
    rowsInsertedSpy.wait(1000);
    compare(api.theModel.rowCount(), 101);
    compare(getData(18).name, "entry 18");
    compare(getData(19).name, "entry 100");
    compare(getData(20).name, "entry 19");
    compare(getData(21).name, "entry 4202");

    rowsInsertedSpy.clear();
    api.insertNewModelItems(30, 10000);
    rowsInsertedSpy.wait(1000);
    compare(rowsInsertedSpy.signalArguments[0][1], 30);
    compare(rowsInsertedSpy.signalArguments[0][2], 10000);
    compare(api.theModel.rowCount(), 10072);
    compare(getData(30).name, "entry 101");

    rowsAboutToBeRemovedSpy.clear();
    rowsRemovedSpy.clear();
    api.deleteModelItems(18, 18);
    rowsAboutToBeRemovedSpy.wait(1000);
    rowsRemovedSpy.wait(1000);
    compare(api.theModel.rowCount(), 10071);
    compare(getData(17).name, "entry 17");
    compare(getData(18).name, "entry 100");
    compare(getData(19).name, "entry 19");
    compare(getData(20).name, "entry 4202");

    rowsRemovedSpy.clear();
    api.deleteModelItems(40, 10010);
    rowsRemovedSpy.wait(1000);
    compare(rowsRemovedSpy.signalArguments[0][1], 40);
    compare(rowsRemovedSpy.signalArguments[0][2], 10010);
    compare(api.theModel.rowCount(), 100);
    compare(getData(39).name, "entry 101");
    compare(getData(40).name, "entry 40");

    rowsRemovedSpy.clear();
    api.deleteModelItems(1, 99);
    rowsRemovedSpy.wait(1000);
    compare(api.theModel.rowCount(), 1);
    compare(getData(0).name, "entry 0");

    rowsRemovedSpy.clear();
    api.deleteModelItems(0, 0);
    rowsRemovedSpy.wait(1000);
    compare(api.theModel.rowCount(), 0);

    rowsInsertedSpy.clear();
    api.insertNewModelItems(0, 199);
    rowsInsertedSpy.wait(1000);
    compare(api.theModel.rowCount(), 200);
    compare(getData(0).name, "entry 102");
}
