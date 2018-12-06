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
import QtQml 2.2
import tuner 1.0

import QtQuick.Controls 1.2

/**
 * QML Implementation of the TunerViewModel API
 */
TunerViewModelImplementationBase {

    id: root

    implementationID: "QML model"

    property int currentStationIndex: 0

    Component.onCompleted: {
        addStation("TSF Jazz", 88000);
        addStation("NRJ", 89500);
        addStation("Big FM", 95500);
        addStation("QML station", 100000);
        trySelectStation(currentStationIndex);
    }

    nextStation: function() {
        trySelectStation(currentStationIndex + 1)
    }

    previousStation: function() {
        trySelectStation(currentStationIndex - 1)
    }

    setenable_AF: function(enabled) {
        print("Request to set enable_AF requested to " + enabled)
        enable_AF = enabled
    }

    setCurrentStation: function(s) {
        print("TODO : implement setCurrentStation " + s)
/*
        var elements = stationList.elements();
        for (var station in elements) {
        }
*/
    }

    function trySelectStation(stationIndex) {
        if ((stationIndex < stationList.size()) && (stationIndex >= 0 )) {
            var station = stationList.elementAt(currentStationIndex)
            currentStationIndex = stationIndex;
            stationList.elementAt(currentStationIndex)
            currentStation = stationList.elementAt(currentStationIndex)
        }
    }

    function addStation(stationName, frequency) {
        var station = StationFactory.create()
        station.name = stationName;
        station.frequency = frequency;
        stationList.addElement(station);
    }

}
