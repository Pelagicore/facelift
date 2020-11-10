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

\page Annotation Annotations

This section provides a description of the annotations which can be used in QFace interface
definition files. An annotation can be used to enable a feature that is disabled by default.


\section Interface_annotations Interface

The following annotations can be used in interface definitions:

|Annotation                     |              Description                                    | Supported over IPC |
|------------------------------ |-------------------------------------------------------------|--------------------|
|\@ipc-sync: true               | Enables the creation of a synchronous/blocking IPC proxy    | Yes                |
|\@ipc-async: true              | Enables the creation of a asynchronous/non-blocking IPC proxy | Yes              |
|\@qml-implementation: true     | Enables the creation and registration of a QML component which can be used to write an implementation of the interface using QML/JS languages | Yes |


\section Structure_annotations Structure

The following annotations can be used in structure definitions:

|Annotation                     |              Description                                    | Supported over IPC |
|-------------------------------|-------------------------------------------------------------|--------------------|
|\@toByteArrayOverDBus: true    | Serialize the whole structure as one byte array over IPC reducing structure signature only to DBUS Type ARRAY OF BYTE | Yes             |
|\@serializable: true           | Adds serialization/deserialization capability to the structure | Yes             |
|\@qml-component: true          | Enables the creation and registration of a creatable QML component for the corresponding structure | Yes |


\section Property_annotations Property

The following annotations can be used in property definitions:

|Annotation                     |              Description                                    | Supported over IPC |
|-------------------------------|-------------------------------------------------------------|--------------------|
|\@hasReadyFlag: true           | See \ref property-ready-flag                                | No                 |


\section Method_annotations Method

The following annotations can be used in method definitions:

|Annotation                     |              Description                                    | Supported over IPC |
|-------------------------------|-------------------------------------------------------------|--------------------|
|\@async: true                  | See \ref async-methods                                      | Yes                |


\section Example_annotations Example

Here is an example QFace file that applies all of the above annotations:
\code
@ipc-async: true
@ipc-sync: true
@qml-implementation: true
interface MyInterface {
    @hasReadyFlag: true
    int readyProperty;

    @async: true
    MyStruct getValueAsync();
}

@serializable: true
@qml-component: true
struct MyStruct
{
    int i;
    string s;
}
\endcode

\section Annotations_interactions Interactions
There are no interactions between annotations.

*/
