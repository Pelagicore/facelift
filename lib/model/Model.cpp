/*
 *   This is part of the QMLCppAPI project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 *   Please see the LICENSE file for details. 
 */

#include "Model.h"

ModelElementID ModelStructure::s_nextID = 0;
constexpr int ModelStructure::ROLE_ID;

QQmlEngine* QMLModelImplementationFrontendBase::s_engine = nullptr;
