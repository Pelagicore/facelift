/**********************************************************************
**
** Copyright (C) Luxoft Sweden AB 2020
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
\page page-properties Properties

\section properties-intro Introduction
Interface definition files (*.qface) can define properties. See https://github.com/Pelagicore/qface for information
about QFace interface definition.

In Facelift each property is exposed to the QML engine in a generated adapter class, as Q_PROPERTY, with setter
and getter functions, and a change signal. If the property is marked as 'readonly' then no setter is generated.
The generated class \<Inerface name\>QMLAdapter wrapps the generated service, which is the owner of the properties.

In Facelift properties are generated as members of the \<Inerface name\>ImplementationBase C++ class.
The facelift::Property class template provides encapsulation for the internal data. To get the underlying value
developer can use a getter that returns the const reference to the underlying data (in case of the service property
it is const reference to a pointer) or one of the specialization specific access functions. The value can be changed by
using a setter, operator= or one of the specialization specific access functions. There is also a set of change operators
provided: ++, \--, -=, +=, *= and /=. Another option is using \ref bindings "the property bindings (see below)". This guarantees
 that the data will not be unnoticibly changed. Each time the setter with a value different than the previous one,
 or other operation that changes the value is called, the change signal is emitted.

\section cpp C++

Each property generated as C++ code is encapsulated inside one of the templated property containers: Property<T>, ListProperty<T>,
ServiceProperty<T> or ModelProperty<T>. Additionally ListProperty and QMap<QString, T> partial template specializations
add extra convenience functions: void removeAt(int i), void addElement(ElementType element) and int size() const.
The QMap<QString, T> specialization is created for the map property defined in *.qface file.

\section subservice Property ownership

The service owns the property objects and maintains their life cycle.

The service property stores a pointer to another service. In this case property object is not an owner of the
pointed service. In terms of QML engine, the service object ownership is held by C++ code.
See https://doc.qt.io/qt-5/qqmlengine.html#ObjectOwnership-enum

Other property types are owners of the underyling objects.


\section bindings Property bindings

Typically the property value change happens on the setter call. An alternative way of updating the property value with a minimal
effort is to bind it to the signal. Whenever the property is notified (the value change triggered), the property will call
a specified getter function. The getter function must return underlying property type.

In the example below, whenever Context object bound to the property emits signal foo(), the property value is set to
the value returned from the lambda passed as bind() parameter.
\code

class Context : public QObject {
  Q_OBJECT
public:
  Q_SIGNAL void fooChanged();
  QString foo() const;
};

class Example : public QObject {
  Q_OBJECT
public:
  facelift::Property<QString> prop;

  void bar(Context* ctx) {
    prop.bind([ctx]() {
        return ctx->foo();
    }).addTrigger(ctx, &Context::fooChanged);
  }
};


\endcode

The change signal emitting rules apply.
Explicit call to the value setter method breaks all bindings and future external signals will have no effect.

\section details Implementation details
The property of the generated interface is implemented as a template container of the property type declared in the
qface file.

facelift::Property supports simple and complex data types. There are three partial specializations for specific data
types: QMap<QString, T>, QList<T> and a pointer of a service object (the service class must be derived from QObject).

Diagram below presents the property classes/templates internal structure

\startuml

class PropertyBase

class "TProperty<Type>" as TProperty_t << (T, #00DDAA) template>> {
#m_value : Type
#m_previousValue : Type
}
class "ModelProperty<ElementType>" as ModelProperty_t << (T, #00DDAA) template>>
class "Model<ElementType>" as Model_t << (T, #00DDAA) template>>
class ModelBase

class "Property<Type>" as Property_t << (T, #00DDAA) template>>

class ChangeSignal << (F,#F0D0C0) Functor >>
class GetterFunction << (F,#F0D0C0) Functor >>

class "ServiceProperty<Type>" as ServiceProperty_t << (T, #00DDAA) template>>
class "Property<Type*,enable_if<is_base_of<QObject*,Type>>>" as Property_service << (S, #AADD00) partial template specialization>>

class "Property<QMap<QString, ElementType>>" as Property_map << (S, #AADD00) partial template specialization>>

class "ListProperty<QList<ElementType>>" as ListProperty_t << (T, #00DDAA) template>>
class "Property<QList<ElementType>>" as Property_list << (S, #AADD00) partial template specialization>>

class Type << (T, #AA33DD) Type from template>>

PropertyBase <|-down- TProperty_t
PropertyBase <|-- ModelProperty_t
PropertyBase o-left- "QObject " : owner object
PropertyBase "1" o-right- "2" ChangeSignal

TProperty_t <|-down- Property_t
TProperty_t <|-- ServiceProperty_t : <<Type*>>
TProperty_t o-left- GetterFunction
TProperty_t "1" *-right- "2" Type
TProperty_t <|-- Property_map
TProperty_t <|-- ListProperty_t

ListProperty_t <|-- Property_list

ServiceProperty_t <|-- Property_service

QObject <|-- ModelBase
ModelBase <|-- Model_t
Model_t <|-- ModelProperty_t : <<ElementType>>


\enduml


*/
