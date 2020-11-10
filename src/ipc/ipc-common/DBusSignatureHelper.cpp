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

#include "DBusSignatureHelper.h"
#include "ipc-common.h"

void facelift::appendPropertySignature(QTextStream& s, const char* propertyName, const char* type, bool readonly)
{
    s << "<property name=\"" << propertyName << "\" type=\"" << type << "\" access=\"" << (readonly ? "read": "readwrite") << "\"/>";
}

void facelift::appendReadyProperty(QTextStream& s)
{
    s << "<property name=\"ready\" type=\"b\" access=\"read\"/>";
}

void facelift::appendDBusMethodSignature(QTextStream &s, const char* methodName, const std::list<std::pair<const char*, const char*>>& inputArgs, const char* outputArgType)
{
    s << "<method name=\"" << methodName << "\">";
    for (const auto& arg: inputArgs) {
        s << "<arg name=\"" << arg.first << "\" type=\"" << arg.second << "\" direction=\"in\"/>";
    }
    if (strlen(outputArgType) != 0) {
        s << "<arg name=\"returnValue\" type=\"" << outputArgType << "\" direction=\"out\"/>";
    }
    s << "</method>";
}

void facelift::appendDBusSignalSignature(QTextStream &s, const char* signalName, const std::list<std::pair<const char*, const char*>>& args)
{
    s << "<signal name=\"" << signalName << "\">";
    for (const auto& arg: args) {
        s << "<arg name=\"" << arg.first << "\" type=\"" << arg.second << "\" direction=\"out\"/>";
    }
    s << "</signal>";
}

void facelift::appendDBusModelSignals(QTextStream &s)
{
    s << "<signal name=\"" << IPCCommon::MODEL_DATA_CHANGED_MESSAGE_NAME << "\">";
    s << "<arg name=\"modelName\" type=\"s\" direction=\"out\"/>";
    s << "<arg name=\"first\" type=\"i\" direction=\"out\"/>";
    s << "<arg name=\"changedItems\" type=\"a(v)\" direction=\"out\"/>";
    s << "</signal>";
    s << "<signal name=\"" << IPCCommon::MODEL_INSERT_MESSAGE_NAME << "\">";
    s << "<arg name=\"modelName\" type=\"s\" direction=\"out\"/>";
    s << "<arg name=\"first\" type=\"i\" direction=\"out\"/>";
    s << "<arg name=\"last\" type=\"i\" direction=\"out\"/>";
    s << "</signal>";
    s << "<signal name=\"" << facelift::IPCCommon::MODEL_REMOVE_MESSAGE_NAME << "\">";
    s << "<arg name=\"modelName\" type=\"s\" direction=\"out\"/>";
    s << "<arg name=\"first\" type=\"i\" direction=\"out\"/>";
    s << "<arg name=\"last\" type=\"i\" direction=\"out\"/>";
    s << "</signal>";
    s << "<signal name=\"" << facelift::IPCCommon::MODEL_MOVE_MESSAGE_NAME << "\">";
    s << "<arg name=\"modelName\" type=\"s\" direction=\"out\"/>";
    s << "<arg name=\"sourceFirstIndex\" type=\"i\" direction=\"out\"/>";
    s << "<arg name=\"sourceLastIndex\" type=\"i\" direction=\"out\"/>";
    s << "<arg name=\"destinationIndex\" type=\"i\" direction=\"out\"/>";
    s << "</signal>";
    s << "<signal name=\"" << facelift::IPCCommon::MODEL_RESET_MESSAGE_NAME << "\">";
    s << "<arg name=\"modelName\" type=\"s\" direction=\"out\"/>";
    s << "<arg name=\"size\" type=\"i\" direction=\"out\"/>";
    s << "</signal>";
}
