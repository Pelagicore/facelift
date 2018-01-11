/**

\page cpp_implementation Implementing an interface in C++

\section ii Implementing an interface in C++

Any interface defined with QFace can be implemented using C++ code.

Let's take an example of interface definition such as the following:
\include "mypackage.qface"

Facelift's code generator generates a C++ abstract class, based on the definition written with the QFace language.
\include "MyInterface.h"

As far as we are concerned now, the most important things to note are the following:
- Any QFace method is turned into a pure virtual C++ method.
- Any QFace property is turned into a pure virtual getter method, and a corresponding "value changed" signal. If the property is not read-only a corresponding setter method is also generated.
- Any QFace signal is turned into a Q_SIGNAL.

Note that this class does not define any Q_INVOKABLE or Q_PROPERTY. This is due to the fact that this class is not meant to be registered directly as a QML type.

In order to provide an implementation of the interface, a new C++ concrete class needs to be defined, which extends the generated abstract class, and provides implementations for all the pure virtual methods.

To make developers' life easier, Facelift code generator generates another kind of class : MyInterfacePropertyAdapter. That class extends the MyInterface class, implements the property getter methods, and
provides convenient "property" members, which can be used to manipulate the property values. Whenever a property value is changed, the corresponding signal is automatically triggered, which can save a lot of
boilerplate code.

Example of implementation based on the PropertyAdapter base class:
\include "MyInterfaceCppImplementation.h"



*/
