/*
 *   This file is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the <license name> licence.
 *   Please see the LICENSE file for details.
 */

#include "FaceliftPlugin.h"

#ifdef ENABLE_DESKTOP_TOOLS
#include "ServiceMonitorQMLComponent.h"
#endif

void FaceliftPlugin::registerTypes(const char *uri)
{
    Q_UNUSED(uri);
#ifdef ENABLE_DESKTOP_TOOLS
    ::qmlRegisterType<facelift::ServiceMonitorQMLComponent>(uri, 1, 0, "ServiceMonitor");
#endif
}

