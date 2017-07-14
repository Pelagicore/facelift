/*
 *   This is part of the QMLCppAPI project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 */

#pragma once

#include <QObject>

#include "widgets.h"

class ServiceMonitorBase : public QObject {

	Q_OBJECT

};

template<typename ProviderType>
class ServiceMonitor : public ServiceMonitorBase {

public:

	ServiceMonitor(ProviderType& provider) : m_provider(provider) {
	}


private:

	ProviderType& m_provider;

};
