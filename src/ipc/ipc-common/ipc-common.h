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

#include <QDataStream>

#include "FaceliftModel.h"
#include "InputPayLoad.h"
#include "OutputPayLoad.h"
#include "DBusSignatureHelper.h"
#include "IPCTypeRegisterHandler.h"

namespace facelift {

Q_DECLARE_LOGGING_CATEGORY(LogIpc)

enum class CommonSignalID {
    readyChanged,
    firstSpecific
};

typedef int ASyncRequestID;

enum class IPCHandlingResult {
    OK,          // Message is successfully handled
    OK_ASYNC,    // Message is handled but it is an asynchronous request, so no reply should be sent for now
    INVALID,     // Message is invalid and could not be handled
};

enum class ModelUpdateEvent {
    DataChanged,
    Insert,
    Remove,
    Move,
    Reset
};

template<typename Type>
inline void assignDefaultValue(Type &v)
{
    v = Type {};
}


}

