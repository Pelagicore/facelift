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

\page MyAppExample Example

This section contains a complete example of application built using Facelift. The examples shown in that page are extracted from an example
application called "MyPackage", which you can launch using the "./examples/launch-mypackage.sh" script.

\section iv QFace interface definition
Let's take an example of interface definition such as the following:
\include "mypackage.qface"

\section vii UI code
The interface definition above already defines completely what any kind of of implementation of that interface is going to expose to the QML UI code,
since no implementation detail should be visible to the UI code.
Here is an example of UI code which could be used in the case where an implementation is registered as a creatable type:
\include "MyApp.qml"


\section iii Interface implementation

There are multiple ways to implement a Facelift interface.

\subsection si Implementation using C++

Facelift's code generator generates a C++ abstract class, based on the definition written in the QFace language:
\include "MyInterface.h"

As far as we are concerned now, the most important things to note are the following:
- Any QFace method is turned into a pure virtual C++ method.
- Any QFace property is turned into a pure virtual getter method, and a corresponding "value changed" signal. If the property is not read-only a corresponding pure virtual setter method is also generated.
- Any QFace signal is turned into a Q_SIGNAL.

Note that this class does not define any Q_INVOKABLE or Q_PROPERTY. This is due to the fact that this class is not meant to be registered directly as a QML type.

In order to provide an implementation of the interface, a new C++ concrete class needs to be defined, which extends the generated abstract class, and provides implementations for all the pure virtual methods.

\subsubsection ssi Implementation using C++ "PropertyAdapter" base class

To make developers' life easier, Facelift code generator generates another kind of class : MyInterfacePropertyAdapter. That class extends the MyInterface class, implements the property getter methods, and
provides convenient "property" members, which can be used to manipulate the property values. Whenever a property value is changed, the corresponding signal is automatically triggered, which can save a lot of
boilerplate code.
\include "MyInterfacePropertyAdapter.h"

Here is a example of implementation based on the PropertyAdapter base class:
\include "MyInterfaceCppImplementation.h"

\section iiii Registration of the interface

Now that our class is implemented, it needs to be registered in order to be imported from the QML code.
As with Qt/QML itself, there are multiple ways to register a type, depending on its nature (singleton, non-singleton, ...)
Note that since a C++ implementation of our interface is a class which is not QML-friendly (it does not expose any Q_PROPERTY or Q_INVOKABLE),
it is not registered directly into the QML engine, using the usual Qt/QML API.

Example:
\include MyPackagePlugin.cpp


*/
