/*
 *   This is part of the QMLCppAPI project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 *   Please see the LICENSE file for details. 
 */

#pragma once

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

template<typename Type> inline void writeJSON(QJsonValue& json, const Type& value) {
    json = value;
}

template<typename Type> inline void writeJSON(QJsonValue& json, const QList<Type>& value) {
    Q_UNUSED(json);
    Q_UNUSED(value);
}

template<typename Type> inline void readJSON(const QJsonValue& json, Type& value) {
    Q_UNUSED(value);
    Q_UNUSED(json);
    Q_ASSERT(false);
}

template<> inline void readJSON<int>(const QJsonValue& json, int& value) {
    if(json.isDouble()) {
        value = json.toInt();
    }
}

template<> inline void readJSON<bool>(const QJsonValue& json, bool& value) {
    if(json.isBool()) {
        value = json.toBool();
    }
    else {
        qFatal("Invalid data type");
    }
}

template<> inline void readJSON<QString>(const QJsonValue& json, QString& value) {
    if(json.isString()) {
        value = json.toString();
    }
    else {
        qFatal("Invalid data type");
    }
}
