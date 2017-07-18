/**

\page Examples

\section BasicExample MyPackage example

This section gives you an overview about how to define an interface, implement it, and use it from your application UI code.

\subsection interface QFace interface definition
\include "mypackage.qface"

\subsection ui-code UI code
\include "MyApp.qml"

\subsection cpp-impl C++ implementation of our interface
\include "MyInterfaceCppImplementation.h"

\subsection qml-impl QML implementation of our interface
\include "MyInterface.qml"

\section IPCExample IPC example

An interface can be easily exposed to an IPC.

\subsection ipc-client-code Server side code
\include "MyAppIPCServer.qml"

\subsection ipc-client-ui-code Client side UI code
\include "MyAppIPCClient.qml"



*/
