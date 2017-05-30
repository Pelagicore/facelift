/*
 *   This file is part of the QMLCppAPI project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the <license name> licence.
 *   Please see the LICENSE file for details.
 */

#include "TunerModelPlugin.h"

#include "tuner/TunerModuleDummy.h"
#include "tuner/TunerModule.h"
#include "tuner/TunerViewModelQML.h"

#include "models/TunerViewModelCpp.h"

void TunerModelPlugin::registerTypes(const char *uri)
{
    TunerModule::registerTypes();
    TunerModule::registerQmlTypes(uri);

    registerQmlComponent<TunerViewModelCpp>(uri);

    TunerModuleDummy::registerQmlTypes(uri);

}
