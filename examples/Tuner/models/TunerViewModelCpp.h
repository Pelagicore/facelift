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

#pragma once

#include "tuner/TunerViewModelImplementationBase.h"
#include "TunerService.h"
#include "FaceliftLogging.h"

using namespace tuner;

class TunerViewModelImplementation :
    public TunerViewModelImplementationBase
{

    Q_OBJECT

public:
    TunerViewModelImplementation(QObject *parent = nullptr) :
        TunerViewModelImplementationBase(parent)
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
                qCDebug(LogGeneral) << station.name;
                qCDebug(LogGeneral) << station.stationId;
                modelStation.setname(station.name);
                modelStation.setstationId(station.stationId);
                modelStation.setprogramType(station.isInfo ? ProgramType::Info : ProgramType::Music);
                modelStation.setisPlaying(m_service.currentStation().stationId == station.stationId);
                modelStationList.push_back(modelStation);
                qCDebug(LogGeneral) << modelStation;
            }
            qCDebug(LogGeneral) << modelStationList;
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
