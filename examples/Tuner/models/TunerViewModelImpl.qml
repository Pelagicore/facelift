import QtQuick 2.0
import QtQml 2.2
import tuner 1.0

import QtQuick.Controls 1.2

/**
 * QML Implementation of the TunerViewModel API
 */
TunerViewModelImplementation {

    implementationID: "QML model"
    currentStationIndex: 1

    Component.onCompleted: {
        addStation("FM1");
        addStation("Bayern 3");
        addStation("Big FM");
        addStation("QML station");
        trySelectStation(0);
    }

    onNextStation: {
        trySelectStation(currentStationIndex+1)
    }

    onPreviousStation: {
        trySelectStation(currentStationIndex-1)
    }

    onUpdateCurrentStation: {
        var index = stationList.elementIndexById(stationId);
        trySelectStation(index);
    }

    function trySelectStation(stationIndex) {
        if ((stationIndex < stationList.size()) && (stationIndex >= 0 )) {
            currentStationIndex = stationIndex;
            currentStation = stationList.elementAt(currentStationIndex)
        }
    }
    
    function addStation(stationName) {
        var station = TunerModule.createStation()
        station.name = stationName;
        stationList.addElement(station);
    }
}
