#pragma once

#include <QObject>

#include "property/Property.h"

class TunerService : public QObject {

	Q_OBJECT

public:

	struct Station {

		int stationId;
		QString name;
		int frequency;

		bool operator==(const Station& s) const {
			if (stationId != s.stationId)
				return false;
			if (name != s.name)
				return false;
			if (frequency != s.frequency)
				return false;
			return true;
		}

	};

	Q_SIGNAL void onCurrentStationChanged();

	Station currentStation() const {
		return m_currentStation;
	}

	Q_SIGNAL void onStationListChanged();

	const QList<Station>& stationList() {
		return m_stationList;
	}

	TunerService() {
		addStation("TSF Jazz");
		addStation("France Info");
		addStation("Big FM");
		setCurrentStationByID(0);
		m_currentStation.init("CurrentStation", this, &TunerService::onCurrentStationChanged);
	}

	void setCurrentStationByFrequency(int frequency) {
		m_currentStation = Station();
		for (const auto& station : m_stationList) {
			if (station.frequency == frequency)
				m_currentStation = station;
		}
	}

	void setCurrentStationByID(int stationID) {
    	qDebug() << "fsggfs1";

    	for (const auto& station : m_stationList) {
        	qDebug() << "fsggfs2";
	if (station.stationId == stationID)
    	qDebug() << "fsggfs3";
				setCurrentStationByFrequency(station.frequency);
		}
	}

private:

	void addStation(const char *stationName) {
		static int nextID = 0;

		Station s;
		s.name = stationName;
		s.stationId = nextID++;
		s.frequency = 88000 + s.stationId * 2100;
		m_stationList.append(s);
	}

	Property<Station> m_currentStation;
	QList<Station> m_stationList;

};
