/*
 *   This file is part of the QMLCppAPI project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the <license name> licence.
 *   Please see the LICENSE file for details.
 */

#include "MediaPlayerPlugin.h"

#include "mediaplayer/Module.h"
#include "mediaplayer/ModuleDummy.h"
#include "models/MediaIndexerModelCpp.h"

using namespace mediaplayer;

void TunerModelPlugin::registerTypes(const char *uri)
{
    mediaplayer::Module::registerTypes();
    mediaplayer::Module::registerQmlTypes(uri);

    registerQmlComponent<MediaIndexerModelCpp>(uri);

    mediaplayer::ModuleDummy::registerQmlTypes(uri);

}
