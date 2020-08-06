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

#include <QObject>

#include "ModelProperty.h"

struct TunerServiceStation
{
public:
    int stationId = 0;
    QString name;
    int frequency = 0;
    bool isInfo = false;

    bool operator==(const TunerServiceStation &s) const
    {
        if (stationId != s.stationId) {
            return false;
        }
        if (name != s.name) {
            return false;
        }
        if (frequency != s.frequency) {
            return false;
        }
        if (isInfo != s.isInfo) {
            return false;
        }
        return true;
    }
};

class TunerService :
    public QObject
{

    Q_OBJECT

public:

    struct RDSSettings
    {
        bool af = true;
    };

    void setAFEnabled(bool enabled)
    {
        m_rdsSettings.af = enabled;
        onRDSettingsChanged();
    }

    Q_SIGNAL void onRDSettingsChanged();

    const RDSSettings &rdsSettings() const
    {
        return m_rdsSettings;
    }

    Q_SIGNAL void onCurrentStationChanged();

    TunerServiceStation currentStation() const
    {
        return m_currentStation;
    }

    Q_SIGNAL void onStationListChanged();

    const QVector<TunerServiceStation> &stationList()
    {
        return m_stationList;
    }

    TunerService()
    {
        addStation("TSF Jazz", false);
        addStation("France Info", true);
        addStation("Big FM", false);
        setCurrentStationByID(0);
    }

    void setCurrentStationByFrequency(int frequency)
    {
        setCurrentStation(TunerServiceStation());
        for (const auto &station : m_stationList) {
            if (station.frequency == frequency) {
                setCurrentStation(station);
            }
        }
    }

    void setCurrentStationByID(int stationID)
    {
        for (const auto &station : m_stationList) {
            if (station.stationId == stationID) {
                setCurrentStationByFrequency(station.frequency);
            }
        }
    }

private:
    void addStation(const char *stationName, bool isInfo)
    {
        static int nextID = 0;

        TunerServiceStation s;
        s.name = stationName;
        s.stationId = nextID++;
        s.isInfo = isInfo;
        s.frequency = 88000 + s.stationId * 2100;
        m_stationList.push_back(s);
    }

    void setCurrentStation(const TunerServiceStation &s)
    {
        if (!(m_currentStation == s))
        {
            m_currentStation = s;
            onCurrentStationChanged();
        }
    }

    TunerServiceStation m_currentStation;
    QVector<TunerServiceStation> m_stationList;
    RDSSettings m_rdsSettings;

};
