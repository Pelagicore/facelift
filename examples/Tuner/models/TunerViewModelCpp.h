#pragma once

#include "tuner/TunerViewModelPropertyAdapter.h"

using namespace tuner;

class TunerViewModelCpp :
    public TunerViewModelPropertyAdapter
{

    Q_OBJECT

public:
    TunerViewModelCpp(QObject *parent = nullptr) :
        TunerViewModelPropertyAdapter(parent)
    {
        setImplementationID("C++ model");

        addStation("FM1");
        addStation("Bayern 3");
        addStation("Big FM");
        addStation("C++ station");
        trySelectStation(0);
    }

    void nextStation() override
    {
        trySelectStation(m_currentStationIndex + 1);
    }

    void previousStation() override
    {
        trySelectStation(m_currentStationIndex - 1);
    }

    void updateCurrentStation(int stationId) override
    {
        qDebug() << "updateCurrentStation called " << stationId;
        for (int index = 0; index < m_stationList.list().size(); index++) {
            const auto &station = m_stationList.list()[index];
            if (station.id() == stationId) {
                trySelectStation(index);
            }
        }
    }

private:
    void trySelectStation(int stationIndex)
    {
        auto &list = m_stationList.list();
        if ((stationIndex < list.size()) && (stationIndex >= 0)) {
            m_currentStationIndex = stationIndex;
            m_currentStation = list[m_currentStationIndex];
        }
    }

    void addStation(const char *stationName)
    {
        Station s;
        s.setname(stationName);
        m_stationList.list().append(s);
    }

};
