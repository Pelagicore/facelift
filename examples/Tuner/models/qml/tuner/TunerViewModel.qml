import QtQuick 2.0
import QtQml 2.2
import tuner 1.0

import QtQuick.Controls 1.2

/**
 * QML Implementation of the TunerViewModel API
 */
TunerViewModelImplementation {

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
        var station = TunerModule.createStation()
        station.name = stationName;
        station.frequency = frequency;
        stationList.addElement(station);
    }

}
