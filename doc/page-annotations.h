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

This section provides a description of the annotations which can be used in QFace interface definition
files. Annotations are typically used to enable some features on some types, if those features are disabled
by default.

\section Structure_annotations Structure

The following annotations can be used in structure definitions:

|Annotation                     |              Description                                    |
|-------------------------------|:-----------------------------------------------------------:|
|\@serializable: true           | Adds serialization/deserialization capability to the structure |
|\@qml-component: true          | Enables the creation and registration of a creatable QML component for the corresponding structure |

\section Interface_annotations Interface

The following annotations can be used in interface definitions:

|Annotation                     |              Description                                    |
|------------------------------ |:-----------------------------------------------------------:|
|\@ipc-sync: true               | Enables the creation of a synchronous/blocking IPC proxy |
|\@ipc-async: true              | Enables the creation of a asynchronous/non-blocking IPC proxy |
|\@qml-implementation: true     | Enables the creation and registration of a QML component which can be used to write an implementation of the interface using QML/JS languages |

*/
