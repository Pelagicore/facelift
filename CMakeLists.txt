cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
project(facelift VERSION 1.0.0)

include(GNUInstallDirs)    # for standard installation locations

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
   set(DEBUG_BUILD ON)
   set(RELEASE_BUILD OFF)
else()
   set(DEBUG_BUILD OFF)
   set(RELEASE_BUILD ON)
endif()

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")

option(FACELIFT_ENABLE_QMLPLUGINDUMP "Enable the generation of plugin type info" OFF)
option(FACELIFT_ENABLE_CODEGEN "Enable code generator build and install. Should typically be disabled when cross-compiling" ON)
option(FACELIFT_ENABLE_DBUS_IPC "Force DBus IPC support (no autodetection)" OFF)
option(FACELIFT_DISABLE_DBUS_IPC "Force disable DBus IPC support (no autodetection)" OFF)
option(FACELIFT_BUILD_EXAMPLES "Enable build of examples" ON)
option(FACELIFT_BUILD_TESTS "Enable build of tests" OFF)
option(FACELIFT_ENABLE_DESKTOP_DEV_TOOLS "Enable desktop development tools" ${DEBUG_BUILD})

if (FACELIFT_ENABLE_DBUS_IPC AND FACELIFT_DISABLE_DBUS_IPC)
    message(FATAL_ERROR "DBus IPC cannot be enabled and disabled at the same time.")
else()
    if (FACELIFT_ENABLE_DBUS_IPC OR (NOT FACELIFT_DISABLE_DBUS_IPC))
        set(FACELIFT_DBUS_IPC_ENABLED ON)
    endif()
endif()

find_package(Qt5 REQUIRED Core Gui Qml Quick)

set(CMAKE_CONFIG_INSTALLATION_PATH ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 14)
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")
endif()

set_property(GLOBAL PROPERTY FACELIFT_CODEGEN_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/codegen)

include("cmake/faceliftMacros.cmake")

set(AUTO_UNITY_BUILD ON)

# No undefined symbol allowed in our shared libraries when using GCC
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
endif()

# Make our build folder look like our installation folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})

function(add_example_script NAME MAIN_QML_FILE)
    set(QML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_QML_FILE})
    if(WIN32)
        configure_file(${PROJECT_SOURCE_DIR}/examples/launch-example.bat.in ${PROJECT_BINARY_DIR}/examples/launch-${NAME}.bat @ONLY)
    else()
        configure_file(${PROJECT_SOURCE_DIR}/examples/launch-example.sh ${PROJECT_BINARY_DIR}/examples/launch-${NAME}.sh @ONLY)
    endif()
endfunction()

if(FACELIFT_ENABLE_CODEGEN)
    add_subdirectory(codegen)
endif()

add_subdirectory(src)

if(FACELIFT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(FACELIFT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

add_subdirectory(doc)

file(GLOB_RECURSE FILES_TO_SHOW_IN_QTCREATOR *.qml *.js *.cmake qmldir)
add_custom_target(FACELIFT_SHOW_IN_QTCREATOR SOURCES ${FILES_TO_SHOW_IN_QTCREATOR})

get_property(FACELIFT_CODEGEN_LOCATION GLOBAL PROPERTY FACELIFT_CODEGEN_LOCATION)
set(FACELIFT_IMPORT_DIRS ${CMAKE_BINARY_DIR}/imports)
configure_file(variables.cmake.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Variables.cmake @ONLY)

set(FACELIFT_CODEGEN_LOCATION \${CMAKE_CURRENT_LIST_DIR}/../../../bin)   # relative path of the installed code generator
set(FACELIFT_IMPORT_DIRS \${CMAKE_CURRENT_LIST_DIR}/../../../imports)
configure_file(variables.cmake.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Variables.cmake.installed @ONLY)

facelift_export_project(BUILD_FILES cmake/faceliftMacros.cmake ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Variables.cmake INSTALLED_FILES cmake/faceliftMacros.cmake ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Variables.cmake.installed)
