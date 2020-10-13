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

if(UNIX AND NOT APPLE)
    # Ensure our shared libraries do not contain any undefined symbol. That is the default on OS X
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
endif()

option(FACELIFT_ENABLE_QMLPLUGINDUMP "Enable the generation of plugin type info" OFF)
option(FACELIFT_ENABLE_CODEGEN "Enable code generator build and install. Should typically be disabled when cross-compiling" ON)
option(FACELIFT_ENABLE_DBUS_IPC "Force DBus IPC support (no autodetection)" OFF)
option(FACELIFT_DISABLE_DBUS_IPC "Force disable DBus IPC support (no autodetection)" OFF)
option(FACELIFT_BUILD_EXAMPLES "Enable build of examples" ON)
option(FACELIFT_BUILD_TESTS "Enable build of tests" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries. If set to off, static libraries are built instead" ON)
option(FACELIFT_DISABLE_GTEST "Disable GTest-based unit tests" OFF)

if (FACELIFT_ENABLE_DBUS_IPC AND FACELIFT_DISABLE_DBUS_IPC)
    message(FATAL_ERROR "DBus IPC cannot be enabled and disabled at the same time.")
else()
    if (FACELIFT_ENABLE_DBUS_IPC OR (NOT FACELIFT_DISABLE_DBUS_IPC))
        set(FACELIFT_DBUS_IPC_ENABLED ON)
    endif()
endif()

set_property(GLOBAL PROPERTY FACELIFT_REQUIRED_QT_MODULES Core Gui Qml Quick)
get_property(REQUIRED_QT_MODULES GLOBAL PROPERTY FACELIFT_REQUIRED_QT_MODULES)

find_package(Qt5 REQUIRED ${REQUIRED_QT_MODULES})

set(CMAKE_CONFIG_INSTALLATION_PATH ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 14)


# Given a list of compiler options, this function returns the list of supported options
function(check_supported_compiler_flags RETURN_VARIABLE_NAME)

    unset(SUPPORTED_COMPILER_FLAGS)

    foreach(COMPILER_FLAG ${ARGN})
        # Create a simple hello world program to compile
        file(WRITE ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/hello.cpp "int main(int, char **) {return 0;}")
        try_compile(RESULT
            ${CMAKE_BINARY_DIR}
            ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/hello.cpp
            COMPILE_DEFINITIONS -Wall -Werror ${COMPILER_FLAG}
        )
        if (RESULT)
            list(APPEND SUPPORTED_COMPILER_FLAGS ${COMPILER_FLAG})
        endif()
    endforeach()

    set(${RETURN_VARIABLE_NAME} ${SUPPORTED_COMPILER_FLAGS} PARENT_SCOPE)

endfunction()

if(NOT MSVC AND NOT ANDROID)
    check_supported_compiler_flags(SUPPORTED_FLAGS
        -Wsuggest-override
        -Wno-gnu-zero-variadic-macro-arguments
        -Wall
        -pedantic
        -Wextra
    )
    string(REPLACE ";" " " SUPPORTED_FLAGS "${SUPPORTED_FLAGS}")
    string(APPEND CMAKE_CXX_FLAGS " ${SUPPORTED_FLAGS}")
    message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
endif()

if(FACELIFT_ENABLE_CODEGEN)
    add_subdirectory(codegen)
endif()

add_subdirectory(cmake)
include("cmake/faceliftMacros.cmake")

set(AUTO_UNITY_BUILD ON)

# Make our build folder look like our installation folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})

function(add_example_script NAME MAIN_QML_FILE)
    set(QML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_QML_FILE})
    configure_file(${PROJECT_SOURCE_DIR}/examples/launch-example.sh ${PROJECT_BINARY_DIR}/examples/launch-${NAME}.sh @ONLY)
endfunction()

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

get_property(REQUIRED_QT_MODULES GLOBAL PROPERTY FACELIFT_REQUIRED_QT_MODULES)

set(FACELIFT_IMPORT_DIRS ${CMAKE_BINARY_DIR}/imports)
configure_file(variables.cmake.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Variables.cmake @ONLY)

set(FACELIFT_IMPORT_DIRS \${CMAKE_CURRENT_LIST_DIR}/../../../imports)
configure_file(variables.cmake.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Variables.cmake.installed @ONLY)

facelift_export_project(
    BUILD_FILES
        cmake/faceliftMacros.cmake
        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Variables.cmake
        ${CMAKE_BINARY_DIR}/faceliftConfig-codegen.cmake
        ${CMAKE_BINARY_DIR}/faceliftUnityConfig.cmake
    INSTALLED_FILES
        cmake/faceliftMacros.cmake
        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Variables.cmake.installed
        ${CMAKE_BINARY_DIR}/faceliftConfig-codegen.installed.cmake
        ${CMAKE_BINARY_DIR}/faceliftUnityConfig.installed.cmake
)
