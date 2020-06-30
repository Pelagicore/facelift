/**

\page "Monolithic build"

\section monolithic-introduction Introduction

In order to execute some code from an executable or a library, the dependencies of that library (functions or data
provided by other libraries) first need to be resolved. That process happens at runtime and typically slows down the
startup of an application.
The CMake functions provided by Facelift implement a mechanism which enables multiple "logical" shared libraries to be
transparently packed together into a single "physical" shared library. Packaging multiple libraries into a single one
is a way to improve application startup time as well as improving the overall application performance since it also enables
link-time optimizations (LTO) to be applied to a larger code base, which enables some more aggressive optimizations
to be applied.


\section monolithic-usage Usage

In order to have multiple libraries packed into a single one, the "MONOLITHIC_SUPPORTED" argument needs to be provided
to the "facelift_add_library" CMake function.

Here is an example of project defining 2 libraries which support monolithic build:
\include "examples/MonolithicBuild/CMakeLists.txt"

Note that calling "facelift_export_project()" is mandatory for projects which make use of the MONOLITHIC_SUPPORTED argument.

The "ENABLE_MONOLITHIC_BUILD" configure option is used to control whether libraries defined with "MONOLITHIC_SUPPORTED" are
built as separate libraries (ENABLE_MONOLITHIC_BUILD=OFF, which is the default value), or built as a single library
(ENABLE_MONOLITHIC_BUILD=ON).


\section monolithic-limitations Limitations

Libraries which are defined with MONOLITHIC_SUPPORTED must be completely defined with a single call to
"facelift_add_library". CMake functions such as "link_libraries()" may not be called and must be replaced by
the corresponding argument of "facelift_add_library" (LINK_LIBRARIES as far as "link_libraries() is concerned).

\section monolithic-requirements Relevant requirements

   - When the ENABLE_MONOLITHIC_BUILD configure option is set to ON, the symbols from all libraries declared with
the MONOLITHIC_SUPPORTED flag should be added to a single shared library.
   - The name of the monolithic library should be the name of the project (the argument of CMake "project()" function).
   - The limitations introduced by enabling MONOLITHIC_SUPPORTED for a library should exist even if the package is not
configured with ENABLE_MONOLITHIC_BUILD=ON. The rational is that developers usually have "ENABLE_MONOLITHIC_BUILD=OFF"
on their development machine, but we want them to face the same limitations as if ENABLE_MONOLITHIC_BUILD was ON,
in order to have any issue fixed as early as possible.

\section monolithic-implementation Implementation details

If MONOLITHIC_SUPPORTED is provided to "facelift_add_library()" when declaring a library called "MyLibrary", multiple
libraries are created under the hood. The definitions of those libraries depend on whether ENABLE_MONOLITHIC_BUILD is 
set to ON or OFF. The only library identifier to be used by the user code is "MyLibrary". All other libraries are
implementation details which are not supposed to be known by Facelift CMake function users.

\subsection monolithic-implementation-on ENABLE_MONOLITHIC_BUILD is ON

In case ENABLE_MONOLITHIC_BUILD is set to ON, the following libraries are created when calling "facelift_add_library(MyLibrary ....)",
if the MONOLITHIC_SUPPORTED flag is provided:
   - An OBJECT library called "MyLibrary_OBJECTS". The following arguments of facelift_add_library() are applied to that
library : PRIVATE_DEFINITIONS, COMPILE_OPTIONS, PRIVATE_INCLUDE_DIRECTORIES, PROPERTIES.
   - An INTERFACE library called "MyLibrary_". The following arguments of facelift_add_library() are applied to that
library : PUBLIC_DEFINITIONS
   - An INTERFACE library called "MyLibrary", which links against the global shared library.

In addition to those 3 libraries which are defined for each call to "facelift_add_library()", a global shared library is defined,
which links against all the XXX_OBJECT libraries, which means it is the only shared library which actually contains the symbols.
The name of that library is the same as the name of the project.

Here is the CMake target dependency graph for the simple example mentioned above:
\dotfile monolithic.dot


\subsection monolithic-implementation-off ENABLE_MONOLITHIC_BUILD is OFF

In case ENABLE_MONOLITHIC_BUILD is set to OFF, the following libraries are created:
   - An OBJECT library called "MyLibrary_OBJECTS", which contains all definitions provided to facelift_add_library().
   - An INTERFACE library called "MyLibrary", which simply links against "MyLibrary_OBJECTS".

Note that the library called "MyLibrary" is defined as an INTERFACE library regardless of the value of the ENABLE_MONOLITHIC_BUILD 
configure option. This means that the limitations which apply to that library do not depend on ENABLE_MONOLITHIC_BUILD.

Here is the CMake target dependency graph for the simple example mentioned above:
\dotfile no-monolithic.dot




*/
