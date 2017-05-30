#pragma once

#include "tuner/TunerViewModelPropertyAdapter.h"
#include "TunerService.h"

using namespace tuner;

class TunerViewModelCpp :
    public TunerViewModelPropertyAdapter
{

    Q_OBJECT

public:

    TunerViewModelCpp(QObject *parent = nullptr) :
        TunerViewModelPropertyAdapter(parent)
    {
        m_currentStation.bind([this] () {
        	auto station = m_service.currentStation();
        	Station s;
        	s.setfrequency(station.frequency);
        	s.setname(station.name);
        	s.setstationId(station.stationId);
        	return s;
        }).connect(&m_service, &TunerService::onCurrentStationChanged);

        m_stationList.bind([this] () {
        	QList<Station> list;
        	return list;
        }).connect(&m_service, &TunerService::onStationListChanged);
    }

    void nextStation() override
    {
        trySelectStation(m_currentStationIndex + 1);
    }

    void previousStation() override
    {
        trySelectStation(m_currentStationIndex - 1);
    }

private:

    void trySelectStation(int stationIndex)
    {
    	qDebug() << "fsggfs";
        auto &list = m_stationList.list();
        if ((stationIndex < list.size()) && (stationIndex >= 0)) {
        	qDebug() << "fsggfs";
        	m_service.setCurrentStationByID(stationList()[stationIndex].stationId());
        }
    }

    int m_currentStationIndex = 0;
    TunerService m_service;

};
