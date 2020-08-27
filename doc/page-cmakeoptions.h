/**********************************************************************
**
** Copyright (C) Luxoft Sweden AB 2019
**
** This file is part of the documentation of the FaceLift project
**
** This work is licensed under the Creative Commons
** Attribution-ShareAlike 4.0 International License. To view a copy of
** this license, visit http://creativecommons.org/licenses/by-sa/4.0/
** or send a letter to Creative Commons, PO Box 1866, Mountain View,
** CA 94042, USA.
**
** SPDX-License-Identifier: CC-BY-SA-4.0
**
**********************************************************************/

/**

\page CMake-Options CMake configuration options

\section cmake-params CMake options

| Option                            | description                                                         |
|-----------------------------------|---------------------------------------------------------------------|
| FACELIFT_DISABLE_DBUS_IPC         | Disables DBUS IPC. Part of the IPC classes is built but there is no DBUS communication. \n Cannot use with FACELIFT_ENABLE_DBUS_IPC |
| FACELIFT_ENABLE_CODEGEN           | Enables code generation. Should be disabled in rare cases, e.g. may be helpful during cross-compiling. |
| FACELIFT_ENABLE_DBUS_IPC          | Enables DBUS IPC. The DBUS IPC classes are build and DBUS communication is functional. \n Cannot use with FACELIFT_DISABLE_DBUS_IPC |
| FACELIFT_BUILD_EXAMPLES           | builds examples attached to project                                 |
| FACELIFT_BUILD_TESTS              | builds tests (can be run by ctest later)                            |


*/
