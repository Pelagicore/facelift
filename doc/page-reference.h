/**

\page Reference

\section Registered QML types

This table contains the types which are registered as creatable QML types.

QML Component name  | Description   | Type
------------------- | ------------- | ------------- |
InterfaceIPCAdapter | This type can be used to register an object of "Interface" type on the IPC bus, so that it can be used remotely. | IPCServiceAdapterBase
InterfaceIPCProxy        | This type is an IPC proxy for the "Interface" type. It connects through the IPC to a server object, which implements the actual logic. Since a proxy object implements the same interface, it can be used as if the interface was implemented locally. | InterfaceQMLFrontend
InterfaceDummy           | This type is an implementation of "Interface" which shows a control GUI where property values can be set manually, and signals can be triggered. It can be useful during development, in order to test the behavior of the UI code under specific conditions, or when an actual implement of the interface is not available yet. | InterfaceQMLFrontend
InterfaceImplementation  | This type is used to provide a QML implementation of "Interface" | InterfaceQMLFrontend
Interface  | This type is typically registered by the plugin initialization code and can be one custom C++ implementation of the interface, or any of the ones provided by the framework (Dummy implementation, IPC proxy, QML implementation, etc...) | InterfaceQMLFrontend


\section Relations Relations between QML types

Note how the InterfaceQMLFrontend and the InterfaceQMLImplementation differ: the latter is used when writing an implementations of an
interface, which means all the properties defined in the IDL are writable, and the signals can be triggered, whereas the properties exposed
by InterfaceQMLFrontend are typically read-only (if they are defined readonly in the IDL definition).


\startuml

abstract class Interface #88CCDD {
    C++ abstract class defining the interface
...
    generated from IDL
}


class InterfaceQMLFrontend #88CCDD {
    Implements the interface exposed to the QML engine, to be used by the UI code
    Turns data from the interface implementation into QML-friendly types
...
    generated from IDL
}


class InterfaceQMLImplementation {
    Used as base component when implementing "Interface" using QML code
}

class InterfaceIPCAdapter {
}

InterfaceIPCAdapter --> "service" Interface

InterfaceQMLFrontend --> "provider" Interface
InterfaceIPCProxy --|> InterfaceQMLFrontend
InterfaceDummy --|> InterfaceQMLFrontend

InterfaceQMLImplementation --> "provider" Interface


\enduml


*/
