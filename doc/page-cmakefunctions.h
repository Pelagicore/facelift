/**

\page CMake-Functions FaceLift CMake API Reference

\section public-functions Functions for FaceLift Users

The following functions are public and are meant to be used by FaceLift users.

\code
facelift_add_interface(<TargetName>
                       INTERFACE_DEFINITION_FOLDER dir
                       [LINK_LIBRARIES lib1 [lib2...]]
                       [IMPORT_FOLDERS folder1 [folder2...]]
                       [MONOLITHIC_SUPPORTED])
\endcode

This function adds a library containing the C++ classes generated from some QFace interface definition files

\param TargetName Name of the library target to be added
\param INTERFACE_DEFINITION_FOLDER The folder containing the QFace interface definition files from which the library should be generated
\param LINK_LIBRARIES List of libraries that will be linked with this interface library
\param IMPORT_FOLDERS List of folders with QFace files that will be imported

<hr><br>

\code
facelift_generateQRC(<OutputFile>
                     INPUT_FOLDERS folder1 [folder2...])
\endcode

Generates a qrc resource file from the given input folders. This function will look for js, qml, ttf, png, sci, json, astcz, tcsh, tesh, gsh, fsh files.

\param OutputFile The qrc resource file that will be generated
\param INPUT_FOLDERS List of folders containing resources to be added

<hr><br>
\code

facelift_add_qml_plugin(<TargetName>
                        URI uri
                        [VERSION version]
                        [OUTPUT_BASE_DIRECTORY dir]
                        [SOURCES src1 [src2...]]
                        [HEADERS hdr1 [hdr2...]]
                        [SOURCES_GLOB [Globbing expressions]...]
                        [HEADERS_GLOB [Globbing expressions]...]
                        [SOURCES_GLOB_RECURSE [Globbing expressions]...]
                        [HEADERS_GLOB_RECURSE [Globbing expressions]...]
                        [HEADERS_NO_INSTALL hdrn1 [hdrn2...]]
                        [HEADERS_GLOB_NO_INSTALL [Globbing expressions]...]
                        [HEADERS_GLOB_RECURSE_NO_INSTALL [Globbing expressions]...]
                        [LINK_LIBRARIES lib1 [lib2...]]
                        [LINK_DEPENDS_NO_SHARED <ON|OFF>]
                        [NO_INSTALL])
\endcode

This function adds a Qml plugin. The parameter descriptions can be found below.

\code

facelift_add_library(<TargetName>
                     [STATIC | SHARED]
                     [SOURCES src1 [src2...]]
                     [HEADERS hdr1 [hdr2...]]
                     [SOURCES_GLOB [Globbing expressions]...]
                     [HEADERS_GLOB [Globbing expressions]...]
                     [SOURCES_GLOB_RECURSE [Globbing expressions]...]
                     [HEADERS_GLOB_RECURSE [Globbing expressions]...]
                     [HEADERS_NO_INSTALL hdrn1 [hdrn2...]]
                     [HEADERS_GLOB_NO_INSTALL [Globbing expressions]...]
                     [HEADERS_GLOB_RECURSE_NO_INSTALL [Globbing expressions]...]
                     [PUBLIC_HEADER_BASE_PATH path]
                     [LINK_LIBRARIES lib1 [lib2...]]
                     [NO_EXPORT]
                     [NO_INSTALL]
                     [LINK_DEPENDS_NO_SHARED <ON|OFF>]
                     [MONOLITHIC_SUPPORTED])
\endcode

This function adds a library with the name \e TargetName from the specified source and header files. The parameter descriptions can be found below.

\code
facelift_add_executable(<TargetName>
                        [SOURCES src1 [src2...]]
                        [HEADERS hdr1 [hdr2...]]
                        [SOURCES_GLOB gsrc1 [Globbing expressions]...]
                        [HEADERS_GLOB ghdr1 [Globbing expressions]...]
                        [SOURCES_GLOB_RECURSE [Globbing expressions]...]
                        [HEADERS_GLOB_RECURSE [Globbing expressions]...]
                        [LINK_LIBRARIES lib1 [lib2...]]
                        [LINK_DEPENDS_NO_SHARED <ON|OFF>])

\endcode

This function adds an executable with the name \e TargetName from the specified source and header files. The parameter descriptions can be found below.

\code
facelift_add_test(<TargetName>
                  [SOURCES src1 [src2...]]
                  [HEADERS hdr1 [hdr2...]]
                  [SOURCES_GLOB [Globbing expressions]...]
                  [HEADERS_GLOB [Globbing expressions]...]
                  [SOURCES_GLOB_RECURSE [Globbing expressions]...]
                  [HEADERS_GLOB_RECURSE [Globbing expressions]...]
                  [LINK_LIBRARIES lib1 [lib2...]]
                  [LINK_DEPENDS_NO_SHARED <ON|OFF>])
\endcode


This function adds a test with the name \e TargetName.The parameter descriptions can be found below.

<hr><br>
\param TargetName The name of the target that will be created
\param SOURCES List of source files used to build this target
\param HEADERS List of header files to be MOCed and added to this target
\param HEADERS_GLOB List of header files to be MOCed and added to this target provided as a wildcard expression
\param SOURCES_GLOB List of source files used to build this target provided as a wildcard expression
\param SOURCES_GLOB_RECURSE List of source files searched recursively inside folder and subfolders, used to build this target provided as a wildcard expression
\param HEADERS_GLOB_RECURSE List of header files to be MOCed and added to this target, searched recursively inside folder and subfolders provided as a wildcard expression
\param LINK_LIBRARIES List of libraries that will be linked with this target
\param HEADERS_NO_INSTALL Same as HEADERS, but those headers wouldn't be installed
\param HEADERS_GLOB_NO_INSTALL Same as HEADERS_GLOB, but those headers wouldn't be installed
\param HEADERS_GLOB_RECURSE_NO_INSTALL Same as HEADERS_GLOB_RECURSE, but those headers wouldn't be installed
\param PUBLIC_HEADER_BASE_PATH The Path to be added to the public include directories of this target
\param NO_EXPORT Prevents from exporting this target
\param NO_INSTALL Prevents from installing this target
\param OUTPUT_BASE_DIRECTORY The directory relative to CMAKE_BINARY_DIR, where the plugin will be installed. If not given default is imports
\param URI The URI of the Target i.e qml plugin
\param VERSION The target i.e Qml plugin version, if not provided it defaults to 1.0
\param MONOLITHIC_SUPPORTED Include the library into the monolithic library
\param LINK_DEPENDS_NO_SHARED Append cmake property LINK_DEPENDS_NO_SHARED to the target

*/
