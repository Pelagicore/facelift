cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
project(facelift VERSION 1.0.0)

include(GNUInstallDirs)    # for standard installation locations

if(NOT DEFINED FACELIFT_ENABLE_IPC)
    message("Detecting if QtDBus is available")
    find_package(Qt5DBus)
endif()

option(FACELIFT_ENABLE_QMLPLUGINDUMP "Enable the generation of plugin type info" OFF)
option(FACELIFT_ENABLE_CODEGEN "Enable code generator build and install. Should typically be disabled when cross-compiling" ON)
option(FACELIFT_ENABLE_IPC "Enable IPC support" ${Qt5DBus_FOUND})
option(FACELIFT_BUILD_EXAMPLES "Enable build of examples" OFF)
option(FACELIFT_BUILD_TEST "Enable build of examples" OFF)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)

if (FACELIFT_ENABLE_IPC)
    message("IPC is enabled => we need Qt5DBus")
    find_package(Qt5DBus REQUIRED)
    add_definitions(-DFACELIFT_ENABLE_IPC)
endif()

set(CMAKE_CONFIG_INSTALLATION_PATH ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

set_property(GLOBAL PROPERTY FACELIFT_CODEGEN_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/codegen)

include("cmake/faceliftMacros.cmake")

# No undefined symbol allowed in our shared libraries when using GCC
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

function(add_example_script NAME MAIN_QML_FILE)
    set(QML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_QML_FILE})
    configure_file(${PROJECT_SOURCE_DIR}/examples/launch-example.sh ${PROJECT_BINARY_DIR}/examples/launch-${NAME}.sh @ONLY)
endfunction()

if(FACELIFT_ENABLE_CODEGEN)
    add_subdirectory(codegen)
endif()

add_subdirectory(lib)

if(FACELIFT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(FACELIFT_BUILD_TEST)
    add_subdirectory(test)
    add_subdirectory(examples/test)
endif()

add_subdirectory(doc)

file(GLOB_RECURSE FILES_TO_SHOW_IN_QTCREATOR *.qml *.js *.cmake qmldir *.qface)
add_custom_target(FACELIFT_SHOW_IN_QTCREATOR SOURCES ${FILES_TO_SHOW_IN_QTCREATOR})

get_property(FACELIFT_CODEGEN_LOCATION GLOBAL PROPERTY FACELIFT_CODEGEN_LOCATION)
configure_file(variables.cmake.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Variables.cmake @ONLY)

set(FACELIFT_CODEGEN_LOCATION \${CMAKE_CURRENT_LIST_DIR}/../../../bin)   # relative path of the installed code generator
configure_file(variables.cmake.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Variables.cmake.installed @ONLY)

facelift_export_project(BUILD_FILES cmake/faceliftMacros.cmake ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Variables.cmake INSTALLED_FILES cmake/faceliftMacros.cmake ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Variables.cmake.installed)
