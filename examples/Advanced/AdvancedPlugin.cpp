/*
 *   This file is part of the FaceLift project
 *   Copyright (C) 2018 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the <license name> licence.
 *   Please see the LICENSE file for details.
 */

#include "AdvancedPlugin.h"

#include "advanced/Module.h"
#include "models/AdvancedModelCpp.h"

void TunerModelPlugin::registerTypes(const char *uri)
{
    advanced::Module::registerQmlTypes(uri);
    facelift::registerSingletonQmlComponent<AdvancedModelCpp>(uri);
}
