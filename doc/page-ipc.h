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

Facelift provides an interprocess communication (IPC) layer which enables objects to interact with each-other across
process boundaries. A typical scenario involves a process which registers a server object, and another process acting
as a client of that server.
The communication over process boundaries currently relies on DBus.

\section ipc-objectregistry Server object registry
Server objects are identified by an object path, which needs to be a valid DBus object path (https://dbus.freedesktop.org/doc/dbus-tutorial.html#objects),
since there is a direct mapping to DBus. Facelift uses a central system-wide registry, which contains entries for all Facelift
server objects registered in the system. The process maintaining the object registry is the first process which either registers
a server object, or creates a proxy object. Given an object path, the object registry can be used by clients to identify the process which
has registered a server under that object path, in order to establish a connection.

\section ipc-secsas Synchronous vs Asynchronous communication

Facelift provides 2 different ways for a client to interact with a server: using synchronous communication (blocking calls) and
using asynchronous communication (non-blocking calls).
The benefit of using synchronous communication is that the client interacts with the server in the same way as if the server was a local
object (registered in the same process as the client). The main drawback of that approach is that the client may be blocked for a long time
in case the server process is not responsive and it can even lead to deadlocks if the server process is also performing a blocking call to
the client process. If the server process is guaranteed to never perform any blocking call and to be responsive, using blocking calls might
be acceptable.
Using asynchronous communication is a way to avoid deadlocks and responsiveness issues, but it is likely to produce a more complex code on the
client side.

\subsection ipc-sync-diff Differences between synchronous and asynchronous IPC proxy classes

The API and behavior of synchronous and asynchronous proxies are similar, but they differ in the following way:
 - Once a synchronous proxy is requested to connect to the server, it immediately tries to establish the connection to the server and, if successful, it
 fetches the property values and can be used immediately. On the other hand, an asynchronous proxy fetches the values of the server in an asynchronous
 way, which means its initial state is always "not ready" and will switch to "ready" only later, once the property values have been fetched successfully.
 - Methods (as defined in the QFace definition) of asynchronous proxies always return "void" but they take an additional "callback function" argument. This
 function is called with the method's return value when the method execution is completed on the server. Note that a QFace method marked with "@async" will
 produce the same method signature in both the synchronous and the asynchronous proxies. In other words, an asynchronous proxy is similar to a synchronous
 proxy where all QFace methods are marked "@async".

\section ipc-annotations IPC interface annotations

An interface can be used over IPC only if it is properly annotated. Two distinct annotations are available:
  - "@ipc-sync: true" : is used to enable the creation of a synchronous IPC proxy.
  - "@ipc-async: true" : is used to enable the creation of a asynchronous IPC proxy.

Note that the server-side IPC code will be generated if at least one of those annotations has been specified.

Example ("MyPackage.qface"):
\snippet "MyPackage.qface" indoc

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
Obviously, \c IPC.enabled: \c true will register this object on the IPC. The \c IPC.objectPath is
optional and in case there is only a single object of this type registered on the system, the default value
should be enough. Otherwise, it can be used to differentiate between several objects of the same type.

\section ipc-client-sync Client side using synchronous proxy

On the client side the replica, which is called \c MyInterfaceIPCProxy in our case is used in the
frontend (UI) QML code. It has the same API as the \c MyInterfaceImplementation on the
server side with an additional property \c ipc:

\snippet MyAppIPCClient.qml indoc

The object path given in \c ipc.objectPath must match the one specified on the server side. The
default value is sufficient if the service is a singleton.


\section ipc-client-async Client side using asynchronous proxy

Asynchronous proxy objects provide a similar interface compared to the synchronous proxies, but the methods
take an additional callback function argument. This function is called when the method execution has been reported by the
server.

\snippet MyAppAsyncIPCClient.qml indoc

\section ipc-sec4 Execution

The server can be started on the command line in the build folder with:
\code
examples/launch-mypackage-ipcserver.sh
\endcode
the synchronous client with:
\code
examples/launch-mypackage-ipcclient.sh
\endcode
the asynchronous client with:
\code
examples/launch-mypackage-ipcclient-async.sh
\endcode
Note how the counter variable increases synchronously in the server and client. Also resetting the
counter by a click on the client window will reset it on the server side, as well.

\section ipc-sec5 Singleton Server

In case the server needs to be based on a singleton implementation instance, it can be provided
through \c MyInterfaceIPCAdapter. This class is registered with the same name in
\c ModuleIPC::registerQmlTypes() (which is called from Module::registerQmlTypes(), if IPC is
enabled). The singleton implementation needs to be registered, too:
\code
facelift::registerSingletonQmlComponent<MyInterfaceImplementation>(uri, "MyInterfaceSingleton");
\endcode
Now the server can be implemented, as follows:
\code
MyInterfaceIPCAdapter {
    service: MyInterfaceSingleton
    enabled: true
}
\endcode
The \c service property of the IPCAdapter has to be set to the interface implementation wich is
provided by the \c MyInterfaceSingleton in this example. Finally, IPC has to be enabled by setting
the \c enabled property of the IPCAdapter to \c true. The optional objectPath could be set, as well.
However, for a singleton the default path should be sufficient. It consists of "/singletons",
followed by the module name followed by the interface name, all separated by slashes and lower
case, in our example: "/singletons/facelift/example/mypackage/myinterface". Of course, the client
IPCProxy needs to refer to the same path in its \c ipc.objectPath property (the default path is
the same).

*/
