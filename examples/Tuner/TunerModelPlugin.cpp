/*
 *   This file is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the <license name> licence.
 *   Please see the LICENSE file for details.
 */

#include "TunerModelPlugin.h"

#include "tuner/Module.h"
#include "tuner/ModuleDummy.h"

#include "models/TunerViewModelCpp.h"

void TunerModelPlugin::registerTypes(const char *uri)
{
    tuner::Module::registerTypes();
    tuner::Module::registerQmlTypes(uri);

    facelift::registerQmlComponent<TunerViewModelCpp>(uri);

    tuner::ModuleDummy::registerQmlTypes(uri);

}
