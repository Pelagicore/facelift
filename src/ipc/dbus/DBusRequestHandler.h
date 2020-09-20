/**********************************************************************
**
** Copyright (C) 2019 Luxoft Sweden AB
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

#if defined(FaceliftIPCLibDBus_LIBRARY)
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_IMPORT
#endif

namespace facelift {
namespace dbus {

class DBusIPCMessage;

class FaceliftIPCLibDBus_EXPORT DBusRequestHandler
{
public:

    virtual void unmarshalProperties(const QVariantMap& changedProperties) = 0;
    virtual void unmarshalPropertiesChanged(const QVariantMap& changedProperties) = 0;
    virtual void handleSignals(DBusIPCMessage& msg) = 0;
    virtual const QList<QString>& getSignals() const = 0;
    virtual void setServiceRegistered(bool isRegistered) = 0;

    virtual ~DBusRequestHandler() = default;
};

} // end namespace dbus
} // end namespace facelift
