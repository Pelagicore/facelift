project(Facelift)

cmake_minimum_required(VERSION 3.1)

option(ENABLE_QMLPLUGINDUMP "Enable the generation of plugin type info" OFF)
option(ENABLE_CODEGEN "Enable code generator build and install. Should typically be disabled when cross-compiling" ON)

include(GNUInstallDirs)    # for standard installation locations

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(Qt5DBus REQUIRED)

set(CMAKE_CONFIG_INSTALLATION_PATH ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

set(CODEGEN_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/codegen)
set(CODEGEN_LOCATION ${CODEGEN_FOLDER}/facelift-codegen.py)
set(QFACE_BASE_LOCATION ${CODEGEN_FOLDER}/facelift/qface)

include("cmake/FaceliftConfig-functions.cmake")

# No undefined symbol allowed in our shared libraries
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/helpers.cmake)

if (ENABLE_CODEGEN)
    add_subdirectory(codegen)
endif()

add_subdirectory(lib)
add_subdirectory(examples)
add_subdirectory(test)

add_subdirectory(doc)

file(GLOB_RECURSE FILES_TO_SHOW_IN_QTCREATOR *.qml *.js *.cmake qmldir *.yaml *.qdl)
add_custom_target(QFACE_SHOW_IN_QTCREATOR SOURCES ${FILES_TO_SHOW_IN_QTCREATOR})

install(FILES cmake/FaceliftConfig-functions.cmake DESTINATION ${CMAKE_CONFIG_INSTALLATION_PATH})
