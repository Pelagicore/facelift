project(facelift)

cmake_minimum_required(VERSION 3.2)

include(GNUInstallDirs)    # for standard installation locations

if(NOT DEFINED FACELIFT_ENABLE_IPC)
    message("Detecting if QtDBus is available")
    find_package(Qt5DBus)
endif()

option(FACELIFT_ENABLE_QMLPLUGINDUMP "Enable the generation of plugin type info" OFF)
option(FACELIFT_ENABLE_CODEGEN "Enable code generator build and install. Should typically be disabled when cross-compiling" ON)
option(FACELIFT_ENABLE_IPC "Enable IPC support" ${Qt5DBus_FOUND})

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

include("cmake/faceliftConfig-functions.cmake")

# No undefined symbol allowed in our shared libraries when using GCC
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/helpers.cmake)

if(FACELIFT_ENABLE_CODEGEN)
    add_subdirectory(codegen)
endif()

add_subdirectory(lib)
add_subdirectory(examples)
add_subdirectory(test)

add_subdirectory(doc)

file(GLOB_RECURSE FILES_TO_SHOW_IN_QTCREATOR *.qml *.js *.cmake qmldir *.yaml *.qdl)
add_custom_target(QFACE_SHOW_IN_QTCREATOR SOURCES ${FILES_TO_SHOW_IN_QTCREATOR})

install(FILES cmake/faceliftConfig-functions.cmake DESTINATION ${CMAKE_CONFIG_INSTALLATION_PATH})
