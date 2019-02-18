
option(IGNORE_AUTO_UNITY_BUILD "Disable unity build even if AUTO_UNITY_BUILD option is ON" OFF)
option(DISABLE_UNITY_BUILD "Completely disable unity build" OFF)

include(GNUInstallDirs)    # for standard installation locations
include(CMakePackageConfigHelpers)

function(facelift_add_unity_files TARGET_NAME VAR_NAME)
    set(FILE_INDEX "0")

    # Limit the number of files per unit to ~ 500Kb, to avoid excessive memory usage
    if(NOT UNITY_BUILD_MAX_FILE_SIZE)
        set(UNITY_BUILD_MAX_FILE_SIZE 256000)
    endif()

    if(NOT UNITY_BUILD_MAX_FILE_COUNT)
        set(UNITY_BUILD_MAX_FILE_COUNT 5)
    endif()

    unset(FILE_LIST)
    unset(NON_UNITY_FILE_LIST)
    foreach(FILE ${ARGN})
        get_filename_component(EXT "${FILE}" EXT)
        get_filename_component(ABSOLUTE_FILE "${FILE}" ABSOLUTE)
        set(EXTENSIONS .h .hpp .cpp)
        list(FIND EXTENSIONS ${EXT} INDEX)
        if(${INDEX} EQUAL -1)
            list(APPEND NON_UNITY_FILE_LIST ${ABSOLUTE_FILE})
        else()
            list(APPEND FILE_LIST ${ABSOLUTE_FILE})
        endif()
    endforeach()

    if(FILE_LIST)
        list(SORT FILE_LIST)
    endif()

    set(AGGREGATED_FILE_LIST "")

    list(LENGTH FILE_LIST REMAINING_FILE_COUNT)

    while(${REMAINING_FILE_COUNT} GREATER 0)

        list(LENGTH FILE_LIST LIST_LENGTH)

        if(NOT LIST_LENGTH)
            break()
        endif()

        math(EXPR FILE_INDEX "${FILE_INDEX}+1")

        set(REMAINING_FILE_COUNT_PER_UNIT ${UNITY_BUILD_MAX_FILE_COUNT})

        unset(FILES)
        set(UNITY_FILE_SIZE 0)
        while((${UNITY_BUILD_MAX_FILE_SIZE} GREATER ${UNITY_FILE_SIZE}) AND (${REMAINING_FILE_COUNT} GREATER 0) AND (${REMAINING_FILE_COUNT_PER_UNIT} GREATER 0))
            list(GET FILE_LIST 0 FILE)
            list(REMOVE_AT FILE_LIST 0)
            list(APPEND FILES ${FILE})

            if(EXISTS ${FILE})
                file(READ "${FILE}" TMP_FILE_CONTENT)
            else()
                unset(TMP_FILE_CONTENT)
            endif()
            string(LENGTH "${TMP_FILE_CONTENT}" FILE_SIZE)

            math(EXPR UNITY_FILE_SIZE "${UNITY_FILE_SIZE}+${FILE_SIZE}")
            math(EXPR REMAINING_FILE_COUNT "${REMAINING_FILE_COUNT}-1")
            math(EXPR REMAINING_FILE_COUNT_PER_UNIT "${REMAINING_FILE_COUNT_PER_UNIT}-1")
        endwhile()

        # Generate an aggregator unit content
        set(FILE_CONTENT "")
        set(FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_unity_${FILE_INDEX}.cpp)
        foreach(SRC_FILE ${FILES})
            set(FILE_CONTENT "${FILE_CONTENT}#include \"${SRC_FILE}\"\n")
        endforeach()

        # To avoid unnecessary recompiles, check if it is really necessary to rewrite the unity file
        if(EXISTS ${FILE_NAME})
            file(READ ${FILE_NAME} OLD_FILE_CONTENT)
        else()
            unset(OLD_FILE_CONTENT)
        endif()

        if(NOT "${OLD_FILE_CONTENT}" STREQUAL "${FILE_CONTENT}")
            file(WRITE ${FILE_NAME} ${FILE_CONTENT})
        endif()

        set_source_files_properties(${FILE_NAME} PROPERTIES OBJECT_DEPENDS "${FILES}")

        list(APPEND AGGREGATED_FILE_LIST ${FILE_NAME})

    endwhile()

    set(${VAR_NAME} ${AGGREGATED_FILE_LIST} ${NON_UNITY_FILE_LIST} PARENT_SCOPE)

endfunction()


# Copy the content of FOLDER_SOURCE into FOLDER_DESTINATION, without overwriting files which already have the same content
function(facelift_synchronize_folders FOLDER_SOURCE FOLDER_DESTINATION)

    file(GLOB_RECURSE SOURCE_FILES RELATIVE ${FOLDER_SOURCE} ${FOLDER_SOURCE}/*)
    foreach(FILE_PATH ${SOURCE_FILES})
        file(READ "${FOLDER_SOURCE}/${FILE_PATH}" FILE_CONTENT)

        if(EXISTS "${FOLDER_DESTINATION}/${FILE_PATH}")
            file(READ "${FOLDER_DESTINATION}/${FILE_PATH}" OLD_CONTENT)
        else()
            set(OLD_CONTENT "")
        endif()

        if (NOT "${FILE_CONTENT}" STREQUAL "${OLD_CONTENT}")
            file(WRITE "${FOLDER_DESTINATION}/${FILE_PATH}" "${FILE_CONTENT}")
        endif()
    endforeach()

    # Remove the files from DESTINATION, which can not be found in SOURCE, so that no old file remains
    file(GLOB_RECURSE DESTINATION_FILES RELATIVE ${FOLDER_DESTINATION} ${FOLDER_DESTINATION}/*)
    foreach(FILE_PATH ${DESTINATION_FILES})
        if(NOT EXISTS "${FOLDER_SOURCE}/${FILE_PATH}")
            file(REMOVE "${FOLDER_DESTINATION}/${FILE_PATH}")
        endif()
    endforeach()

endfunction()


macro(facelift_load_variables)
    set(FACELIFT_GENERATED_HEADERS_INSTALLATION_LOCATION ${CMAKE_INSTALL_INCLUDEDIR}/FaceliftGenerated)
endmacro()


macro(facelift_module_to_libname VAR_NAME QFACE_MODULE_NAME)
    string(REPLACE "." "_" ${VAR_NAME} ${QFACE_MODULE_NAME})
    set(${VAR_NAME} "FaceliftGenerated_${${VAR_NAME}}")
endmacro()


# Deprecated
function(facelift_add_package TARGET_NAME QFACE_MODULE_NAME INTERFACE_FOLDER)
    facelift_add_interface(${TARGET_NAME} INTERFACE_DEFINITION_FOLDER ${INTERFACE_FOLDER})
endfunction()


function(facelift_generate_code )

    set(options GENERATE_ALL)
    set(oneValueArgs INTERFACE_DEFINITION_FOLDER OUTPUT_PATH LIBRARY_NAME)
    set(multiValueArgs IMPORT_FOLDERS)
    cmake_parse_arguments(ARGUMENT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    get_property(CODEGEN_LOCATION GLOBAL PROPERTY FACELIFT_CODEGEN_LOCATION)
    set(QFACE_BASE_LOCATION ${CODEGEN_LOCATION}/facelift/qface)
    set(CODEGEN_EXECUTABLE_LOCATION ${CODEGEN_LOCATION}/facelift-codegen.py)

    file(TO_NATIVE_PATH "${QFACE_BASE_LOCATION}" QFACE_BASE_LOCATION_NATIVE_PATH)
    set(ENV{PYTHONPATH} "${QFACE_BASE_LOCATION_NATIVE_PATH}")

    set(WORK_PATH ${CMAKE_CURRENT_BINARY_DIR}/facelift_generated_tmp)
    file(MAKE_DIRECTORY ${WORK_PATH})

    set(BASE_CODEGEN_COMMAND ${CODEGEN_EXECUTABLE_LOCATION} --input "${ARGUMENT_INTERFACE_DEFINITION_FOLDER}" --output "${WORK_PATH}")

    foreach(IMPORT_FOLDER ${ARGUMENT_IMPORT_FOLDERS})
        list(APPEND BASE_CODEGEN_COMMAND "--dependency" "${IMPORT_FOLDER}")
    endforeach()

    if(ARGUMENT_LIBRARY_NAME)
        list(APPEND BASE_CODEGEN_COMMAND "--library" "${ARGUMENT_LIBRARY_NAME}")
    endif()

    if(ARGUMENT_GENERATE_ALL)
        list(APPEND BASE_CODEGEN_COMMAND "--all")
    endif()

    # find_package(PythonInterp) causes some issues if the another version has been searched before, and it is not needed anyway on non-Win32 platforms
    if(WIN32)
        find_package(PythonInterp 3.0 REQUIRED)
    endif()

    string(REPLACE ";" " " BASE_CODEGEN_COMMAND_WITH_SPACES "${BASE_CODEGEN_COMMAND}")
    message("Calling facelift code generator. Command:\n PYTHONPATH=$ENV{PYTHONPATH} ${PYTHON_EXECUTABLE} ${BASE_CODEGEN_COMMAND_WITH_SPACES}")

    if(NOT DEFINED ENV{LANG}) # e.g. Qt Creator was started from the Apple Dock
        set(ENV{LANG} "en_US.UTF-8")
        # --> http://click.pocoo.org/5/python3/#python-3-surrogate-handling
        # --> https://apple.stackexchange.com/questions/54765/how-can-i-have-qt-creator-to-recognize-my-environment-variables
        set(LANG_SET_BY_FACELIFT true)
    else()
        set(LANG_SET_BY_FACELIFT false)
    endif()

    execute_process(COMMAND ${PYTHON_EXECUTABLE} ${BASE_CODEGEN_COMMAND}
        RESULT_VARIABLE CODEGEN_RETURN_CODE
        WORKING_DIRECTORY ${QFACE_BASE_LOCATION}/qface
        OUTPUT_VARIABLE CODEGEN_OUTPUT
        ERROR_VARIABLE CODEGEN_ERROR
    )

    if(LANG_SET_BY_FACELIFT)
        unset(ENV{LANG})
    endif()

    if(NOT "${CODEGEN_RETURN_CODE}" STREQUAL "0")
        message(FATAL_ERROR "Facelift code generation failed!\nCommand: ${BASE_CODEGEN_COMMAND_WITH_SPACES} with PYTHONPATH=$ENV{PYTHONPATH}\nReturn code: ${CODEGEN_RETURN_CODE}\nOutput: ${CODEGEN_OUTPUT}\nError: ${CODEGEN_ERROR}\n")
    endif()

    facelift_synchronize_folders(${WORK_PATH} ${ARGUMENT_OUTPUT_PATH})

    # Delete work folder
    file(REMOVE_RECURSE ${WORK_PATH})

    # Add a dependency so that CMake will reconfigure whenever one of the interface files is changed, which will refresh our generated files
    file(GLOB_RECURSE QFACE_FILES ${ARGUMENT_INTERFACE_DEFINITION_FOLDER}/*.qface)
    file(GLOB_RECURSE CODEGEN_FILES ${CODEGEN_LOCATION}/*)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CODEGEN_FILES};${QFACE_FILES}")

endfunction()


function(facelift_add_interface TARGET_NAME)

    set(options GENERATE_ALL)
    set(oneValueArgs INTERFACE_DEFINITION_FOLDER)
    set(multiValueArgs IMPORT_FOLDERS LINK_LIBRARIES)
    cmake_parse_arguments(ARGUMENT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    facelift_load_variables()
    set(LIBRARY_NAME ${TARGET_NAME})

    set(GENERATED_HEADERS_INSTALLATION_LOCATION ${FACELIFT_GENERATED_HEADERS_INSTALLATION_LOCATION}/${LIBRARY_NAME})

    if(WIN32)
        set(OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/facelift_generated/${LIBRARY_NAME})  # There is a weird issue on Windows related to the MOC if the generated files are outside of ${CMAKE_CURRENT_BINARY_DIR}
    else()
        set(OUTPUT_PATH ${CMAKE_BINARY_DIR}/facelift_generated/${LIBRARY_NAME})  # Keep generated file folder outside of CMAKE_CURRENT_BINARY_DIR to avoid having the MOC generated file inside the same folder, which would cause unnecessary recompiles
    endif()

    set(TYPES_OUTPUT_PATH ${OUTPUT_PATH}/types)
    set(DEVTOOLS_OUTPUT_PATH ${OUTPUT_PATH}/devtools)
    set(IPC_OUTPUT_PATH ${OUTPUT_PATH}/ipc)
    set(IPC_DBUS_OUTPUT_PATH ${OUTPUT_PATH}/ipc_dbus)
    set(MODULE_OUTPUT_PATH ${OUTPUT_PATH}/module)

    unset(ADDITIONAL_ARGUMENTS)
    if (ARGUMENT_GENERATE_ALL)
        set(ADDITIONAL_ARGUMENTS "GENERATE_ALL")
    endif()

    facelift_generate_code(INTERFACE_DEFINITION_FOLDER ${ARGUMENT_INTERFACE_DEFINITION_FOLDER} IMPORT_FOLDERS ${ARGUMENT_IMPORT_FOLDERS} OUTPUT_PATH ${OUTPUT_PATH} LIBRARY_NAME ${TARGET_NAME} ${ADDITIONAL_ARGUMENTS})

    # Get the list of generated files
    facelift_add_library(${LIBRARY_NAME}_types
        SOURCES_GLOB_RECURSE ${TYPES_OUTPUT_PATH}/*.cpp
        HEADERS_GLOB_RECURSE ${TYPES_OUTPUT_PATH}/*.h
        LINK_LIBRARIES FaceliftModelLib ${ARGUMENT_LINK_LIBRARIES}
        PUBLIC_HEADER_BASE_PATH ${TYPES_OUTPUT_PATH}
        UNITY_BUILD
    )

    facelift_add_library(${LIBRARY_NAME}
        SOURCES_GLOB_RECURSE ${MODULE_OUTPUT_PATH}/*.cpp
        HEADERS_GLOB_RECURSE ${MODULE_OUTPUT_PATH}/*.h
        LINK_LIBRARIES ${LIBRARY_NAME}_types ${ARGUMENT_LINK_LIBRARIES}
        PUBLIC_HEADER_BASE_PATH ${MODULE_OUTPUT_PATH}
        UNITY_BUILD
    )

    if(TARGET FaceliftDesktopDevTools)
        facelift_add_library(${LIBRARY_NAME}_desktop_dev_tools
            SOURCES_GLOB_RECURSE ${DEVTOOLS_OUTPUT_PATH}/*.cpp
            HEADERS_GLOB_RECURSE ${DEVTOOLS_OUTPUT_PATH}/*.h
            LINK_LIBRARIES ${LIBRARY_NAME}_types FaceliftDesktopDevTools ${ARGUMENT_LINK_LIBRARIES}
            PUBLIC_HEADER_BASE_PATH ${DEVTOOLS_OUTPUT_PATH}
            UNITY_BUILD
        )
        target_link_libraries(${LIBRARY_NAME} ${LIBRARY_NAME}_desktop_dev_tools)
        list(APPEND MODULE_COMPILE_DEFINITIONS ENABLE_DESKTOP_TOOLS)
    endif()

    if(TARGET FaceliftIPCLib)
        facelift_add_library(${LIBRARY_NAME}_ipc
            SOURCES_GLOB_RECURSE ${IPC_OUTPUT_PATH}/*.cpp
            HEADERS_GLOB_RECURSE ${IPC_OUTPUT_PATH}/*.h
            LINK_LIBRARIES ${LIBRARY_NAME}_types FaceliftIPCLib ${ARGUMENT_LINK_LIBRARIES}
            PUBLIC_HEADER_BASE_PATH ${IPC_OUTPUT_PATH}
            UNITY_BUILD
        )
        target_link_libraries(${LIBRARY_NAME} ${LIBRARY_NAME}_ipc)
        list(APPEND MODULE_COMPILE_DEFINITIONS ENABLE_IPC)

        if(TARGET FaceliftIPCLibDBus)
            facelift_add_library(${LIBRARY_NAME}_ipc_dbus
                SOURCES_GLOB_RECURSE ${IPC_DBUS_OUTPUT_PATH}/*.cpp
                HEADERS_GLOB_RECURSE ${IPC_DBUS_OUTPUT_PATH}/*.h
                LINK_LIBRARIES ${LIBRARY_NAME}_types FaceliftIPCLib ${ARGUMENT_LINK_LIBRARIES}
                PUBLIC_HEADER_BASE_PATH ${IPC_DBUS_OUTPUT_PATH}
                UNITY_BUILD
            )
            target_link_libraries(${LIBRARY_NAME}_ipc ${LIBRARY_NAME}_ipc_dbus)
        endif()
    endif()

    target_compile_definitions(${LIBRARY_NAME} PRIVATE "${MODULE_COMPILE_DEFINITIONS}")

    # Get the list of files from the interface definition folder so that we can regenerate the code whenever there is a change there
    file(GLOB_RECURSE QFACE_FILES ${ARGUMENT_INTERFACE_DEFINITION_FOLDER}/*.qface)
    target_sources(${LIBRARY_NAME} PRIVATE ${QFACE_FILES})

endfunction()


#function(facelift_set_qml_implementation_path LIBRARY_NAME PATH)
# TODO : fix the way QML implementations are located
#    set_target_properties(${LIBRARY_NAME}_api PROPERTIES
#        COMPILE_DEFINITIONS "QML_MODEL_LOCATION=${PATH}"
#    )
#endfunction()


macro(_facelift_add_target_start)

    set(options NO_INSTALL UNITY_BUILD NO_EXPORT INTERFACE STATIC SHARED USE_QML_COMPILER)
    set(oneValueArgs )
    set(multiValueArgs PRIVATE_DEFINITIONS
        HEADERS HEADERS_GLOB HEADERS_GLOB_RECURSE
        SOURCES SOURCES_GLOB SOURCES_GLOB_RECURSE
        HEADERS_NO_INSTALL HEADERS_GLOB_NO_INSTALL HEADERS_GLOB_RECURSE_NO_INSTALL
        PUBLIC_HEADER_BASE_PATH
        LINK_LIBRARIES
        UI_FILES
        RESOURCE_FOLDERS
    )
    cmake_parse_arguments(ARGUMENT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(SOURCES ${ARGUMENT_SOURCES})
    foreach(SOURCE_GLOB ${ARGUMENT_SOURCES_GLOB_RECURSE})
        file(GLOB_RECURSE GLOB_FILES ${SOURCE_GLOB})
        set(SOURCES ${SOURCES} ${GLOB_FILES})
    endforeach()
    foreach(SOURCE_GLOB ${ARGUMENT_SOURCES_GLOB})
        file(GLOB GLOB_FILES ${SOURCE_GLOB})
        set(SOURCES ${SOURCES} ${GLOB_FILES})
    endforeach()

    set(HEADERS ${ARGUMENT_HEADERS})
    foreach(HEADER_GLOB ${ARGUMENT_HEADERS_GLOB_RECURSE})
        file(GLOB_RECURSE GLOB_FILES ${HEADER_GLOB})
        list(APPEND HEADERS ${GLOB_FILES})
    endforeach()
    foreach(HEADER_GLOB ${ARGUMENT_HEADERS_GLOB})
        file(GLOB GLOB_FILES ${HEADER_GLOB})
        list(APPEND HEADERS ${GLOB_FILES})
    endforeach()

    set(HEADERS_NO_INSTALL ${ARGUMENT_HEADERS_NO_INSTALL})
    foreach(HEADER_GLOB ${ARGUMENT_HEADERS_GLOB_RECURSE_NO_INSTALL})
        file(GLOB_RECURSE GLOB_FILES ${HEADER_GLOB})
        list(APPEND HEADERS_NO_INSTALL ${GLOB_FILES})
    endforeach()
    foreach(HEADER_GLOB ${ARGUMENT_HEADERS_GLOB_NO_INSTALL})
        file(GLOB GLOB_FILES ${HEADER_GLOB})
        list(APPEND HEADERS_NO_INSTALL ${GLOB_FILES})
    endforeach()

    if(ARGUMENT_RESOURCE_FOLDERS)
        set(GENERATED_QRC_PATH "${CMAKE_CURRENT_BINARY_DIR}/resources.qrc")
        facelift_generateQRC("${GENERATED_QRC_PATH}" INPUT_FOLDERS ${ARGUMENT_RESOURCE_FOLDERS})
        list(APPEND RESOURCE_FILES "${GENERATED_QRC_PATH}")
    endif()

    if(ARGUMENT_USE_QML_COMPILER)
        qtquick_compiler_add_resources(RESOURCES_BIN ${RESOURCE_FILES})
    else()
        qt5_add_resources(RESOURCES_BIN ${RESOURCE_FILES})
    endif()

    set(HEADERS_TO_BE_MOCCED ${HEADERS} ${HEADERS_NO_INSTALL})
    unset(HEADERS_MOCS)

    if(HEADERS_TO_BE_MOCCED)
        qt5_wrap_cpp(HEADERS_MOCS ${HEADERS} ${HEADERS_NO_INSTALL} TARGET ${TARGET_NAME})
    endif()

    unset(UI_FILES)
    if(ARGUMENT_UI_FILES)
        qt5_wrap_ui(UI_FILES ${ARGUMENT_UI_FILES})
    endif()

    set(ALL_SOURCES ${SOURCES} ${HEADERS_MOCS} ${UI_FILES} ${RESOURCES_BIN})

    if(NOT DISABLE_UNITY_BUILD)
        if("${AUTO_UNITY_BUILD}" AND NOT "${IGNORE_AUTO_UNITY_BUILD}")
            set(UNITY_BUILD ON)
        else()
            set(UNITY_BUILD ${ARGUMENT_UNITY_BUILD})
        endif()
    endif()

    if(ARGUMENT_USE_QML_COMPILER OR CMAKE_AUTOMOC)
        #qml compiler adds private definitions of V4 to RESOURCES BIN
        #combining set of resources in one file leads to redefinition error message
        set(UNITY_BUILD OFF)
    endif()

    if(UNITY_BUILD)
        facelift_add_unity_files(${TARGET_NAME} ALL_SOURCES "${ALL_SOURCES}")
    endif()

endmacro()

macro(_facelift_add_target_finish)

    if(NOT __INTERFACE)

        target_compile_definitions(${TARGET_NAME} ${__INTERFACE} PRIVATE ${ARGUMENT_PRIVATE_DEFINITIONS})

        # create a valid preprocessor macro base on the target name
        string(REPLACE "-" "_" LIB_PREPROCESSOR_DEFINITION "${TARGET_NAME}_LIBRARY")
        target_compile_definitions(${TARGET_NAME} PRIVATE ${LIB_PREPROCESSOR_DEFINITION})

    endif()

    # We assume every lib links against QtCore at least
    target_link_libraries(${TARGET_NAME} ${__INTERFACE} Qt5::Core ${ARGUMENT_LINK_LIBRARIES})

endmacro()

function(facelift_add_library TARGET_NAME)

    _facelift_add_target_start(${ARGN})

    unset(__INTERFACE)
    if(ARGUMENT_INTERFACE)
        add_library(${TARGET_NAME} INTERFACE)
        set(__INTERFACE INTERFACE)
    else()
       if(NOT ALL_SOURCES)
           set(EMPTY_FILE_PATH ${CMAKE_BINARY_DIR}/facelift_empty_file.cpp)
           if(NOT EXISTS ${EMPTY_FILE_PATH})
              file(WRITE ${EMPTY_FILE_PATH} "#include <QObject>\nQ_DECL_EXPORT void dummy_facelift_function() {}")
           endif()
           set(ALL_SOURCES ${EMPTY_FILE_PATH})
       endif()
        if(${ARGUMENT_STATIC})
            set(LIBRARY_TYPE STATIC)
        endif()
        if(${ARGUMENT_SHARED})
            set(LIBRARY_TYPE SHARED)
        endif()
    add_library(${TARGET_NAME} ${LIBRARY_TYPE} ${ALL_SOURCES})
    endif()

    if (NOT ${ARGUMENT_NO_INSTALL})
        set(INSTALL_LIB ON)
    endif()

    if (NOT ${ARGUMENT_NO_EXPORT})
        set(EXPORT_LIB ON)
    endif()

    if(NOT ARGUMENT_PUBLIC_HEADER_BASE_PATH)
        set(PUBLIC_HEADER_BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
    else()
        set(PUBLIC_HEADER_BASE_PATH ${ARGUMENT_PUBLIC_HEADER_BASE_PATH})
    endif()

    get_filename_component(ABSOLUTE_HEADER_BASE_PATH "${PUBLIC_HEADER_BASE_PATH}" ABSOLUTE)

    if (EXPORT_LIB)

        set(HEADERS_INSTALLATION_LOCATION ${PROJECT_NAME}/${TARGET_NAME})

        # Create the directory for the installed headers
        install(DIRECTORY DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${HEADERS_INSTALLATION_LOCATION})

        # Install every headers
        foreach(HEADER ${HEADERS})
            get_filename_component(ABSOLUTE_HEADER_PATH ${HEADER} ABSOLUTE)
            file(RELATIVE_PATH relativePath "${PUBLIC_HEADER_BASE_PATH}" ${ABSOLUTE_HEADER_PATH})
            get_filename_component(ABSOLUTE_HEADER_INSTALLATION_DIR ${CMAKE_INSTALL_INCLUDEDIR}/${HEADERS_INSTALLATION_LOCATION}/${relativePath} DIRECTORY)
            install(FILES ${HEADER} DESTINATION ${ABSOLUTE_HEADER_INSTALLATION_DIR})
        endforeach()

        if(__INTERFACE)
        else()
            # Do not define target include directories if no headers are present. This avoids the creation and inclusion of empty directories.
            if(HEADERS)
                # Set the installed headers location
                target_include_directories(${TARGET_NAME}
                    PUBLIC
                        $<BUILD_INTERFACE:${ABSOLUTE_HEADER_BASE_PATH}>
                )
            endif()
        endif()

    endif()

    if(INSTALL_LIB)

        # Install library
        install(TARGETS ${TARGET_NAME} EXPORT ${PROJECT_NAME}Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

        if(__INTERFACE)
        else()
            # Do not define target include directories if no headers are present. This avoids the creation and inclusion of empty directories.
            if(HEADERS)
                # Set the installed headers location
                target_include_directories(${TARGET_NAME}
                    PUBLIC
                        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${HEADERS_INSTALLATION_LOCATION}>
                )
            endif()
        endif()

    endif()

    _facelift_add_target_finish()

endfunction()


function(facelift_add_executable TARGET_NAME)
    _facelift_add_target_start(${ARGN})
    add_executable(${TARGET_NAME} ${ALL_SOURCES})

    if (NOT ${ARGUMENT_NO_INSTALL})
        install(TARGETS ${TARGET_NAME} EXPORT ${PROJECT_NAME}Targets DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()

    _facelift_add_target_finish()
endfunction()


function(facelift_add_test TEST_NAME)

    facelift_add_executable(${TEST_NAME} ${ARGN})

    set_target_properties(${TEST_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    )

    add_test(
        NAME ${TEST_NAME}
        COMMAND $<TARGET_FILE:${TEST_NAME}>
    )

endfunction()


# Export this package into CMake's global registry
# This allows users of our package to use the "find_package(PACKAGE_NAME)" statement
# to retrieve the definitions needed to build against our package
function(facelift_export_project)

    set(options)
    set(oneValueArgs )
    set(multiValueArgs BUILD_FILES INSTALLED_FILES)
    cmake_parse_arguments(ARGUMENT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(CMAKE_CONFIG_INSTALLATION_PATH ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

    set(CONFIG_DESTINATION_PATH "${CMAKE_BINARY_DIR}")
    set(VERSION_FILE_PATH ${CONFIG_DESTINATION_PATH}/${PROJECT_NAME}ConfigVersion.cmake)
    write_basic_package_version_file(${VERSION_FILE_PATH} COMPATIBILITY SameMajorVersion)
    install(FILES ${VERSION_FILE_PATH} DESTINATION ${CMAKE_CONFIG_INSTALLATION_PATH})

    # Create a file containing our target definitions
    export(EXPORT ${PROJECT_NAME}Targets FILE ${CONFIG_DESTINATION_PATH}/${PROJECT_NAME}Targets.cmake)

    # Install a similar file in the installation folder
    install(EXPORT ${PROJECT_NAME}Targets DESTINATION ${CMAKE_CONFIG_INSTALLATION_PATH})

    # Create a PackageConfig.cmake in the build folder
    file(WRITE ${CONFIG_DESTINATION_PATH}/${PROJECT_NAME}Config.cmake "")
    foreach(FILE ${ARGUMENT_BUILD_FILES} )
        file(COPY ${FILE} DESTINATION ${CMAKE_BINARY_DIR})
        get_filename_component(INCLUDED_FILE "${FILE}" NAME)
        file(APPEND ${CONFIG_DESTINATION_PATH}/${PROJECT_NAME}Config.cmake "include(\${CMAKE_CURRENT_LIST_DIR}/${INCLUDED_FILE})\n")
    endforeach()
    file(APPEND ${CONFIG_DESTINATION_PATH}/${PROJECT_NAME}Config.cmake "include(\${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}Targets.cmake)\n")

    file(WRITE ${CONFIG_DESTINATION_PATH}/${PROJECT_NAME}Config.cmake.installed "")
    foreach(FILE ${ARGUMENT_INSTALLED_FILES})
        install(FILES ${FILE} DESTINATION ${CMAKE_CONFIG_INSTALLATION_PATH})
        get_filename_component(INCLUDED_FILE "${FILE}" NAME)
        file(APPEND ${CONFIG_DESTINATION_PATH}/${PROJECT_NAME}Config.cmake.installed "include(\${CMAKE_CURRENT_LIST_DIR}/${INCLUDED_FILE})\n")
    endforeach()
    file(APPEND ${CONFIG_DESTINATION_PATH}/${PROJECT_NAME}Config.cmake.installed "include(\${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}Targets.cmake)\n")

    install(FILES ${CONFIG_DESTINATION_PATH}/${PROJECT_NAME}Config.cmake.installed DESTINATION ${CMAKE_CONFIG_INSTALLATION_PATH} RENAME ${PROJECT_NAME}Config.cmake)

endfunction()


function(facelift_append_to_file_if_not_existing FILE_PATH CONTENT)
    file(READ "${FILE_PATH}" FILE_CONTENT)
    string(FIND "${FILE_CONTENT}" "${CONTENT}" IS_FOUND)
    if(${IS_FOUND} LESS 0)
        file(APPEND ${FILE_PATH} "${CONTENT}")
    endif()
endfunction()


function(facelift_add_qml_plugin_qmldir)

    set(options SINGLETONS)
    set(oneValueArgs URI BASE_FILE_PATH VERSION)
    set(multiValueArgs FILES_GLOB_RECURSE)
    cmake_parse_arguments(ARGUMENT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(URI ${ARGUMENT_URI})
    set(BASE_PATH ${ARGUMENT_BASE_FILE_PATH})
    string(REPLACE "." "/" PLUGIN_PATH ${ARGUMENT_URI})
    set(INSTALL_PATH imports/${PLUGIN_PATH})
    set(QMLDIR_INSTALLED_PATH ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/qmldir.installed)
    set(QMLDIR_PATH ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/qmldir)

    if(ARGUMENT_VERSION)
        string(REPLACE "." ";" VERSION_LIST "${ARGUMENT_VERSION}")
        list(GET VERSION_LIST 0 PLUGIN_MAJOR_VERSION)
        list(GET VERSION_LIST 1 PLUGIN_MINOR_VERSION)
    endif()

    file(GLOB_RECURSE qmlSources ${ARGUMENT_FILES_GLOB_RECURSE})

    if(NOT EXISTS ${QMLDIR_INSTALLED_PATH})
        file(WRITE ${QMLDIR_INSTALLED_PATH} "")
    endif()
    if(NOT EXISTS ${QMLDIR_PATH})
        file(WRITE ${QMLDIR_PATH} "")
    endif()
    foreach(qmlSource ${qmlSources})
        get_filename_component(_fileName ${qmlSource} NAME_WE)
        file(RELATIVE_PATH relativePath ${BASE_PATH} ${qmlSource})
        set(LINE_INSTALLED "${_fileName} ${PLUGIN_MAJOR_VERSION}.${PLUGIN_MINOR_VERSION} qrc:/${relativePath}\n")
        file(RELATIVE_PATH relativeSourcePath ${CMAKE_BINARY_DIR}/${INSTALL_PATH} ${qmlSource})
        set(LINE_NOT_INSTALLED "${_fileName} ${PLUGIN_MAJOR_VERSION}.${PLUGIN_MINOR_VERSION} ${relativeSourcePath}\n")
        if(ARGUMENT_SINGLETONS)
            set(LINE_INSTALLED "singleton ${LINE_INSTALLED}")
            set(LINE_NOT_INSTALLED "singleton ${LINE_NOT_INSTALLED}")
        endif()
        facelift_append_to_file_if_not_existing(${QMLDIR_INSTALLED_PATH} "${LINE_INSTALLED}")
        facelift_append_to_file_if_not_existing(${QMLDIR_PATH} "${LINE_NOT_INSTALLED}")
    endforeach()

    install(FILES ${QMLDIR_INSTALLED_PATH} DESTINATION ${INSTALL_PATH} COMPONENT ${PROJECT_NAME} RENAME qmldir)

endfunction()


# Add a FILE and install it at the given DESTINATION in both the build folder and in the installation folder
# The DESTINATION parameter must be a relative path
# If specified, the RELATIVE_PATH_VARIABLE_NAME variable is set in the caller's scope and contains the path where the
# file is located, relative to the RELATIVE_PATH_BASE parameter. That relative path is valid in both the build folder
# and the installation folder
function(facelift_install_file)
    set(options )
    set(oneValueArgs FILE DESTINATION RELATIVE_PATH_BASE RELATIVE_PATH_VARIABLE_NAME)
    set(multiValueArgs )
    cmake_parse_arguments(ARGUMENT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    get_filename_component(BASENAME ${ARGUMENT_FILE} NAME)
    configure_file(${ARGUMENT_FILE} ${CMAKE_BINARY_DIR}/${ARGUMENT_DESTINATION}/${BASENAME} @ONLY)
    install(FILES ${CMAKE_BINARY_DIR}/${ARGUMENT_DESTINATION}/${BASENAME} DESTINATION ${ARGUMENT_DESTINATION})

    if(ARGUMENT_RELATIVE_PATH_VARIABLE_NAME)
       file(RELATIVE_PATH RELATIVE_PATH_VALUE "${CMAKE_INSTALL_PREFIX}/${ARGUMENT_RELATIVE_PATH_BASE}" "${CMAKE_INSTALL_PREFIX}/${ARGUMENT_DESTINATION}/${BASENAME}")
       set(${ARGUMENT_RELATIVE_PATH_VARIABLE_NAME} ${RELATIVE_PATH_VALUE} PARENT_SCOPE)
    endif()

endfunction()


# Build and install a QML plugin
function(facelift_add_qml_plugin PLUGIN_NAME)

    set(options )
    set(oneValueArgs URI VERSION OUTPUT_BASE_DIRECTORY)
    set(multiValueArgs )
    cmake_parse_arguments(ARGUMENT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(URI ${ARGUMENT_URI})

    if(NOT ARGUMENT_VERSION)
        # Default to version 1.0 if no version is specified
        set(ARGUMENT_VERSION "1.0")
    endif()

    if(NOT ARGUMENT_OUTPUT_BASE_DIRECTORY)
        set(ARGUMENT_OUTPUT_BASE_DIRECTORY "imports")
    endif()

    string(REPLACE "." ";" VERSION_LIST "${ARGUMENT_VERSION}")
    list(GET VERSION_LIST 0 PLUGIN_MAJOR_VERSION)
    list(GET VERSION_LIST 1 PLUGIN_MINOR_VERSION)

    string(REPLACE "." "/" PLUGIN_PATH ${URI})

    facelift_add_library(${PLUGIN_NAME} ${ARGUMENT_UNPARSED_ARGUMENTS} NO_INSTALL NO_EXPORT LINK_LIBRARIES Qt5::Qml)

    get_target_property(TARGET_TYPE ${PLUGIN_NAME} TYPE)
    if (TARGET_TYPE STREQUAL "STATIC_LIBRARY")
        target_compile_definitions(${PLUGIN_NAME} PRIVATE QT_STATICPLUGIN)
    endif ()

    target_compile_definitions(${PLUGIN_NAME} PRIVATE "PLUGIN_MINOR_VERSION=${PLUGIN_MINOR_VERSION};PLUGIN_MAJOR_VERSION=${PLUGIN_MAJOR_VERSION}")

    set(INSTALL_PATH ${ARGUMENT_OUTPUT_BASE_DIRECTORY}/${PLUGIN_PATH})

    set_target_properties(${PLUGIN_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${INSTALL_PATH}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${INSTALL_PATH}
    )

    install(TARGETS ${PLUGIN_NAME} DESTINATION ${INSTALL_PATH})

    install(FILES ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/qmldir.installed DESTINATION ${INSTALL_PATH} RENAME qmldir)
    set(QMLDIR_CONTENT "module ${URI}\nplugin ${PLUGIN_NAME}\ntypeinfo plugins.qmltypes\n")
    file(WRITE ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/qmldir "${QMLDIR_CONTENT}")
    file(WRITE ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/qmldir.installed "${QMLDIR_CONTENT}")

    if(NOT CMAKE_CROSSCOMPILING AND NOT WIN32)
        # not supported for now on Win32 since the required libraries can't be loaded without setting the PATH variable
        add_custom_command(
            OUTPUT  ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/plugins.qmltypes
            COMMAND ${_qt5Core_install_prefix}/bin/qmlplugindump -noinstantiate ${URI} ${PLUGIN_MAJOR_VERSION}.${PLUGIN_MINOR_VERSION} ${CMAKE_BINARY_DIR}/imports -output ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/plugins.qmltypes || touch ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/plugins.qmltypes
            DEPENDS ${PLUGIN_NAME}
        )
        add_custom_target("generate_qmltypes_${PLUGIN_NAME}" ALL DEPENDS ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/plugins.qmltypes)
        install(FILES ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/plugins.qmltypes DESTINATION ${INSTALL_PATH})
    endif()

endfunction()


function(facelift_generateQRC OUTPUT_FILE)

    set(options )
    set(oneValueArgs )
    set(multiValueArgs INPUT_FOLDERS)
    cmake_parse_arguments(ARGUMENT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(QRC "<RCC>\n")

    foreach(INPUT_PATH ${ARGUMENT_INPUT_FOLDERS})

        string(APPEND QRC "\t<qresource prefix=\"/\">\n")
        file(GLOB_RECURSE FILES "${INPUT_PATH}/*.qml"
                                "${INPUT_PATH}/*.js"
                                "${INPUT_PATH}/*.ttf"
        )
        foreach(FILE ${FILES})
            file(RELATIVE_PATH RELATIVE_PATH ${INPUT_PATH} ${FILE})
            string(APPEND QRC "\t\t<file alias=\"${RELATIVE_PATH}\">${FILE}</file>\n")
        endforeach()
        string(APPEND QRC "\t</qresource>\n")

        string(APPEND QRC "\t<qresource prefix=\"/images\">\n")
        file(GLOB_RECURSE FILES "${INPUT_PATH}/*.png"
                                "${INPUT_PATH}/*.sci"
                                "${INPUT_PATH}/*.json"
                                "${INPUT_PATH}/*.astcz"
                                "${INPUT_PATH}/*.tcsh"
                                "${INPUT_PATH}/*.tesh"
                                "${INPUT_PATH}/*.gsh"
                                "${INPUT_PATH}/*.fsh"
        )

        foreach(FILE ${FILES})
            file(RELATIVE_PATH RELATIVE_PATH ${INPUT_PATH} ${FILE})
            string(APPEND QRC "\t\t<file alias=\"${RELATIVE_PATH}\">${FILE}</file>\n")
        endforeach()
        string(APPEND QRC "\t</qresource>\n")

    endforeach()

    string(APPEND QRC "</RCC>")

    # Write content to file if not already
    unset(OLD_FILE_CONTENT)
    if(EXISTS ${OUTPUT_FILE})
        file(READ ${OUTPUT_FILE} OLD_FILE_CONTENT)
    endif()
    if(NOT "${OLD_FILE_CONTENT}" STREQUAL "${QRC}")
        file(WRITE "${OUTPUT_FILE}" "${QRC}")
    endif()

endfunction()
