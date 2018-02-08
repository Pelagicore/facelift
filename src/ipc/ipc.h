/*
 *   Copyright (C) 2017 Pelagicore AG
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 *   Please see the LICENSE file for details.
 */

#pragma once

#include "ipc-common/ipc-common.h"

#include "Property.h"

#if 1
#include "dbus/ipc-dbus.h"

namespace facelift {

using IPCMessage = ::facelift::dbus::DBusIPCMessage;
using IPCProxyBinder = ::facelift::dbus::DBusIPCProxyBinder;
using IPCAttachedPropertyFactory = ::facelift::dbus::DBusIPCAttachedPropertyFactory;
using IPCAdapterFactoryManager = ::facelift::dbus::DBusIPCAdapterFactoryManager;

template<typename InterfaceType>
using IPCServiceAdapter = ::facelift::dbus::DBusIPCServiceAdapter<InterfaceType>;

template<typename InterfaceType1, typename InterfaceType2>
using IPCProxy = ::facelift::dbus::DBusIPCProxy<InterfaceType1, InterfaceType2>;

}

#else

#include "local/ipc-local.h"

namespace facelift {

template<typename Type>
using IPCServiceAdapter = LocalIPCServiceAdapter<Type>;
template<typename AdapterType, typename IPCAdapterType>
using IPCProxy = LocalIPCProxy<AdapterType, IPCAdapterType>;
typedef LocalIPCMessage IPCMessage;
typedef LocalIPCAttachedPropertyFactory IPCAttachedPropertyFactory;
typedef LocalIPCAdapterFactoryManager IPCAdapterFactoryManager;
using IPCProxyBinder = ::facelift::LocalIPCProxyBinder;

}

#endif
