/**********************************************************************
**
** Copyright (C) Luxoft Sweden AB 2018
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

\page page-ipc IPC

\section ipc-sec1 Introduction

Facelift provides interprocess communication (IPC) that allows to synchronize objects between
processes. In this context one process acts as a server (usually the System-UI) and the other as a
client (an application). The server side is where the interface is implemented and on the client
side an exact replica is provided.

Facelift also abstracts away the difference between in- and out-of-process
communication. Communication over process boundaries relies on DBus. However, should the client and
server live in the same process after all, direct calls will be made. This is done transparently
for the user.

\section ipc-sec2 Server Side

Let's have a second look at the \ref MyAppExample and see how this simple interface can be used
over IPC. The object that is replicated is the one exposed to the frontend (UI) QML code, the one
derived from MyInterfaceQmlFrontend. In our example it is exposed as \c MyInterfaceImplementation.
In the server side UI code it is basically used as before:

\snippet MyAppIPCServer.qml indoc

The only addition needed is the part that establishes IPC for this object. This is done by means of
an attached property called \c IPC:
\code
        IPC.enabled: true
        IPC.objectPath: "/my/object/path"
\endcode
Obviously \c IPC.enabled: \c true will enable IPC for this object. The \c IPC.objectPath is
optional and in case there is only a single object of this type present on the server side not
needed. Otherwise, it can be used to differentiate between several objects of the same type.

\section ipc-sec3 Client Side

On the client side the replica, which is called \c MyInterfaceIPCProxy in our case is used in the
frontend (UI) QML code. It has the same API as the \c MyInterfaceImplementation on the
server side with an additional property \c ipc:

\snippet MyAppIPCClient.qml indoc

The object path given in \c ipc.objectPath must match the one specified on the server side. The
default value is sufficient if the service is a singleton.

\section ipc-sec4 Execution

The server can be started on the command line in the build folder with:
\code
examples/launch-mypackage-ipcserver.sh
\endcode
and the client with:
\code
examples/launch-mypackage-ipcclient.sh
\endcode
Note how the counter variable increases synchronously in the server and client. Also resetting the
counter by a click on the client window will reset it on the server side, as well.
*/
