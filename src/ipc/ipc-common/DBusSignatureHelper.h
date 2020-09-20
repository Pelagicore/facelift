/**********************************************************************
**
** Copyright (C) 2020 Luxoft Sweden AB
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

#if defined(FaceliftIPCCommonLib_LIBRARY)
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_IMPORT
#endif

#include <QTextStream>

namespace facelift {

void appendPropertySignature(QTextStream& s, const char* propertyName, const char* type, bool readonly);
void appendReadyProperty(QTextStream& s);
void appendDBusMethodSignature(QTextStream &s, const char* methodName, const std::list<std::pair<const char*, const char*>>& inputArgs, const char* outputArgType = "");
void appendDBusSignalSignature(QTextStream &s, const char* signalName, const std::list<std::pair<const char*, const char*>>& args);
void appendDBusModelSignals(QTextStream &s);
}
