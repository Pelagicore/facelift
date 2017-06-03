/*
 *   This file is part of the QMLCppAPI project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the <license name> licence.
 *   Please see the LICENSE file for details.
 */

#include "MediaPlayerPlugin.h"

#include "mediaplayer/MediaplayerModuleDummy.h"
#include "mediaplayer/MediaplayerModule.h"
#include "mediaplayer/MediaIndexerModelDummy.h"
#include "models/MediaIndexerModelCpp.h"

using namespace mediaplayer;

void TunerModelPlugin::registerTypes(const char *uri)
{
    MediaplayerModule::registerTypes();
    MediaplayerModule::registerQmlTypes(uri);

    registerQmlComponent<MediaIndexerModelCpp>(uri);

    MediaplayerModuleDummy::registerQmlTypes(uri);

}
