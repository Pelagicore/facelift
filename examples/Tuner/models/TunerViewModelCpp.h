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

            // Update our station index
            for (int i = 0; i < m_stationList.value().size(); i++) {
                if (m_stationList.value()[i].stationId() == station.stationId) {
                    m_currentStationIndex = i;
                }
            }

            Station modelStation;
            modelStation.setfrequency(station.frequency);
            modelStation.setname(station.name);
            modelStation.setprogramType(station.isInfo ? ProgramType::Info : ProgramType::Music);
            modelStation.setstationId(station.stationId);
            return modelStation;
        }).addTrigger(&m_service, &TunerService::onCurrentStationChanged);

        m_stationList.bind([this] () {
            QList<Station> modelStationList;

            for (const auto &station: m_service.stationList()) {
                Station modelStation;
                qDebug() << station.name;
                qDebug() << station.stationId;
                modelStation.setname(station.name);
                modelStation.setstationId(station.stationId);
                modelStation.setprogramType(station.isInfo ? ProgramType::Info : ProgramType::Music);
                modelStation.setisPlaying(m_service.currentStation().stationId == station.stationId);
                modelStationList.push_back(modelStation);
                qDebug() << modelStation;
            }
            qDebug() << modelStationList;
            return modelStationList;
        }).addTrigger(&m_service, &TunerService::onStationListChanged).addTrigger(&m_service,
                &TunerService::onCurrentStationChanged);

        m_enable_AF.bind([this] () {
            return m_service.rdsSettings().af;
        }).addTrigger(&m_service, &TunerService::onRDSettingsChanged);

    }

    void setenable_AF(const bool &enabled) override
    {
        m_service.setAFEnabled(enabled);
    }

    void nextStation() override
    {
        trySelectStation(m_currentStationIndex + 1);
    }

    void previousStation() override
    {
        trySelectStation(m_currentStationIndex - 1);
    }

    void setCurrentStation(Station station) override
    {
        m_service.setCurrentStationByID(station.stationId());
    }

private:
    void trySelectStation(int stationIndex)
    {
        auto &list = m_stationList.value();
        if ((stationIndex < list.size()) && (stationIndex >= 0)) {
            m_service.setCurrentStationByID(stationList()[stationIndex].stationId());
        }
    }

    int m_currentStationIndex = 0;
    TunerService m_service;

};
