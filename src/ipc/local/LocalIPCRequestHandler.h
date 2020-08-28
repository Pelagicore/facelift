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

#if defined(FaceliftIPCLocalLib_LIBRARY)
#  define FaceliftIPCLocalLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCLocalLib_EXPORT Q_DECL_IMPORT
#endif

#include <memory>

#include <QDebug>

#include "FaceliftModel.h"
#include "FaceliftUtils.h"
#include "ModelProperty.h"
#include "ipc-common.h"

namespace facelift {

namespace ipc { namespace local {
class ObjectRegistry;
class ObjectRegistryAsync;
} }

namespace local {

using namespace facelift;

class FaceliftIPCLocalLib_EXPORT LocalIPCRequestHandler
{

public:
    virtual void unmarshalPropertyValues(LocalIPCMessage &msg) = 0;
    virtual void unmarshalPropertiesChanged(LocalIPCMessage &msg) = 0;
    virtual void handleSignals(LocalIPCMessage& msg) = 0;
    virtual const QList<QString>& getSignals() const = 0;
    virtual void setServiceRegistered(bool isRegistered) = 0;

};


}

}
