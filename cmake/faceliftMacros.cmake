
include(GNUInstallDirs)    # for standard installation locations
include(CMakePackageConfigHelpers)

function(facelift_add_unity_files VAR_NAME)
    set(FILE_INDEX "0")

    unset(FILE_LIST)
    foreach(FILE ${ARGN})
        get_filename_component(ABSOLUTE_FILE "${FILE}" ABSOLUTE)
        set(FILE_LIST ${FILE_LIST} ${ABSOLUTE_FILE})
    endforeach()

    set(AGGREGATED_FILE_LIST "")

    while(1)

        list(LENGTH FILE_LIST LIST_LENGTH)

        if(NOT LIST_LENGTH)
            break()
        endif()

        math(EXPR FILE_INDEX "${FILE_INDEX}+1")

        # Limit the number of files per aggregator compilation unit to 12, to avoid excessive memory usage
        if(LIST_LENGTH GREATER 12)
            list(GET FILE_LIST 0 1 2 3 4 5 6 7 8 9 10 11 FILES)
        else()
            set(FILES ${FILE_LIST})
        endif()

        list(REMOVE_ITEM FILE_LIST ${FILES})

        # Generate an aggregator unit content
        set(FILE_CONTENT "")
        set(FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}_unity_${FILE_INDEX}.cpp)
        foreach(SRC_FILE ${FILES})
            set(FILE_CONTENT "${FILE_CONTENT}#include \"${SRC_FILE}\"\n")
        endforeach()

        # To avoid unnecessary recompiles, check if it is really necessary to rewrite the unity file
        if(EXISTS ${FILE_NAME})
            file(READ ${FILE_NAME} OLD_FILE_CONTENT)
        endif()

        if(NOT "${OLD_FILE_CONTENT}" STREQUAL "${FILE_CONTENT}")
            file(WRITE ${FILE_NAME} ${FILE_CONTENT})
        endif()

        set_source_files_properties(${FILE_NAME} PROPERTIES OBJECT_DEPENDS "${FILES}")

        list(APPEND AGGREGATED_FILE_LIST ${FILE_NAME})

    endwhile()

    set(${VAR_NAME} ${AGGREGATED_FILE_LIST} PARENT_SCOPE)

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


function(facelift_add_interface TARGET_NAME)

    set(options)
    set(oneValueArgs INTERFACE_DEFINITION_FOLDER)
    set(multiValueArgs)
    cmake_parse_arguments(ARGUMENT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    facelift_load_variables()
    set(LIBRARY_NAME ${TARGET_NAME})
    set(INTERFACE_DEFINITION_FOLDER ${ARGUMENT_INTERFACE_DEFINITION_FOLDER})

    set(GENERATED_HEADERS_INSTALLATION_LOCATION ${FACELIFT_GENERATED_HEADERS_INSTALLATION_LOCATION}/${LIBRARY_NAME})

    get_property(CODEGEN_LOCATION GLOBAL PROPERTY FACELIFT_CODEGEN_LOCATION)
    set(QFACE_BASE_LOCATION ${CODEGEN_LOCATION}/facelift/qface)
    set(CODEGEN_EXECUTABLE_LOCATION ${CODEGEN_LOCATION}/facelift-codegen.py)

    file(TO_NATIVE_PATH "${QFACE_BASE_LOCATION}" QFACE_BASE_LOCATION_NATIVE_PATH)
    set(ENV{PYTHONPATH} "${QFACE_BASE_LOCATION_NATIVE_PATH}")

    set(WORK_PATH ${CMAKE_CURRENT_BINARY_DIR}/facelift_generated_tmp)

    set(OUTPUT_PATH ${CMAKE_BINARY_DIR}/facelift_generated/${LIBRARY_NAME})  # Keep generated file folder outside of CMAKE_CURRENT_BINARY_DIR to avoid having the MOC generated file inside the same folder, which would cause unnecessary recompiles
    set(TYPES_OUTPUT_PATH ${OUTPUT_PATH}/types)
    set(DEVTOOLS_OUTPUT_PATH ${OUTPUT_PATH}/devtools)
    set(IPC_OUTPUT_PATH ${OUTPUT_PATH}/ipc)
    set(MODULE_OUTPUT_PATH ${OUTPUT_PATH}/module)

    file(MAKE_DIRECTORY ${WORK_PATH})

    find_package(PythonInterp 3.0 REQUIRED)

    set(CODEGEN_COMMAND ${PYTHON_EXECUTABLE} ${CODEGEN_EXECUTABLE_LOCATION} ${INTERFACE_DEFINITION_FOLDER} ${WORK_PATH})

    string(REPLACE ";" " " CODEGEN_COMMAND_WITH_SPACES "${CODEGEN_COMMAND}")
    message("Calling facelift code generator. Command:\n" ${CODEGEN_COMMAND_WITH_SPACES})

    execute_process(COMMAND ${CODEGEN_COMMAND}
        RESULT_VARIABLE CODEGEN_RETURN_CODE
        WORKING_DIRECTORY ${QFACE_BASE_LOCATION}/qface
    )

    if(NOT "${CODEGEN_RETURN_CODE}" STREQUAL "0")
        message(FATAL_ERROR "Facelift code generation failed. Command \"${CODEGEN_COMMAND_WITH_SPACES}\". PYTHONPATH=$ENV{PYTHONPATH} Return code: ${CODEGEN_RETURN_CODE}\n")
    endif()

    facelift_synchronize_folders(${WORK_PATH} ${OUTPUT_PATH})

    # Delete work folder
    file(REMOVE_RECURSE ${WORK_PATH})

    # Get the list of generated files
    facelift_add_library(${LIBRARY_NAME}_types
        SOURCES_GLOB_RECURSE ${TYPES_OUTPUT_PATH}/*.cpp
        HEADERS_GLOB_RECURSE ${TYPES_OUTPUT_PATH}/*.h
        LINK_LIBRARIES FaceliftModelLib FaceliftQMLModelLib FaceliftPropertyLib
        PUBLIC_HEADER_BASE_PATH ${TYPES_OUTPUT_PATH}
        UNITY_BUILD
    )

    facelift_add_library(${LIBRARY_NAME}
        SOURCES_GLOB_RECURSE ${MODULE_OUTPUT_PATH}/*.cpp
        HEADERS_GLOB_RECURSE ${MODULE_OUTPUT_PATH}/*.h
        LINK_LIBRARIES ${LIBRARY_NAME}_types
        PUBLIC_HEADER_BASE_PATH ${MODULE_OUTPUT_PATH}
        UNITY_BUILD
    )

    if(TARGET FaceliftDesktopDevTools)
        facelift_add_library(${LIBRARY_NAME}_desktop_dev_tools
            SOURCES_GLOB_RECURSE ${DEVTOOLS_OUTPUT_PATH}/*.cpp
            HEADERS_GLOB_RECURSE ${DEVTOOLS_OUTPUT_PATH}/*.h
            LINK_LIBRARIES ${LIBRARY_NAME}_types FaceliftDesktopDevTools
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
            LINK_LIBRARIES ${LIBRARY_NAME}_types FaceliftIPCLib
            PUBLIC_HEADER_BASE_PATH ${IPC_OUTPUT_PATH}
            UNITY_BUILD
        )
        target_link_libraries(${LIBRARY_NAME} ${LIBRARY_NAME}_ipc)
        list(APPEND MODULE_COMPILE_DEFINITIONS ENABLE_IPC)
    endif()

    set_target_properties(${LIBRARY_NAME} PROPERTIES COMPILE_DEFINITIONS "${MODULE_COMPILE_DEFINITIONS}")

    # Add a dummy target to make the QFace files visible in the IDE
    file(GLOB_RECURSE QFACE_FILES ${INTERFACE_DEFINITION_FOLDER}/*.qface)
    add_custom_target(FaceliftPackage_${LIBRARY_NAME} SOURCES ${QFACE_FILES})

endfunction()


#function(facelift_set_qml_implementation_path LIBRARY_NAME PATH)
# TODO : fix the way QML implementations are located
#    set_target_properties(${LIBRARY_NAME}_api PROPERTIES
#        COMPILE_DEFINITIONS "QML_MODEL_LOCATION=${PATH}"
#    )
#endfunction()


function(facelift_add_library LIB_NAME)

    set(options OPTIONAL NO_INSTALL UNITY_BUILD NO_EXPORT INTERFACE)
    set(oneValueArgs )
    set(multiValueArgs HEADERS HEADERS_GLOB HEADERS_GLOB_RECURSE SOURCES SOURCES_GLOB SOURCES_GLOB_RECURSE PUBLIC_HEADER_BASE_PATH LINK_LIBRARIES UI_FILES RESOURCE_FOLDERS)
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
        set(HEADERS ${HEADERS} ${GLOB_FILES})
    endforeach()
    foreach(HEADER_GLOB ${ARGUMENT_HEADERS_GLOB})
        file(GLOB GLOB_FILES ${HEADER_GLOB})
        set(HEADERS ${HEADERS} ${GLOB_FILES})
    endforeach()

    unset(RESOURCE_FILES)
    foreach(RESOURCE_FOLDER ${ARGUMENT_RESOURCE_FOLDERS})
        generateQRC("${RESOURCE_FOLDER}/" "${CMAKE_CURRENT_BINARY_DIR}")
        qt5_add_resources(RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/resources.qrc)
        set(RESOURCE_FILES ${RESOURCE_FILES} ${RESOURCES})
        message("HHHH ${RESOURCE_FILES}")
    endforeach()

    qt5_wrap_cpp(HEADERS_MOCS ${HEADERS} TARGET ${LIB_NAME})

    unset(UI_FILES)
    if(ARGUMENT_UI_FILES)
        qt5_wrap_ui(UI_FILES ${ARGUMENT_UI_FILES})
    endif()

    set(ALL_SOURCES ${SOURCES} ${HEADERS_MOCS} ${UI_FILES} ${RESOURCE_FILES})

    set(UNITY_BUILD ${ARGUMENT_UNITY_BUILD})
    if(${AUTO_UNITY_BUILD})
        set(UNITY_BUILD ON)
    endif()

    if(UNITY_BUILD)
        facelift_add_unity_files(ALL_SOURCES "${ALL_SOURCES}")
    endif()

    unset(__INTERFACE)
    if(ARGUMENT_INTERFACE)
        message("adding interface library : ${LIB_NAME}")
        add_library(${LIB_NAME} INTERFACE)
        set(__INTERFACE INTERFACE)
    else()
       add_library(${LIB_NAME} SHARED ${ALL_SOURCES})
       if(NOT ALL_SOURCES)
          # Assume we have a C type of library if no source is built
          set_target_properties(${LIB_NAME} PROPERTIES LINKER_LANGUAGE C)
       endif()
    endif()

    # We assume every lib links against QtCore at least
    target_link_libraries(${LIB_NAME} ${__INTERFACE} Qt5::Core ${ARGUMENT_LINK_LIBRARIES})

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

        set(HEADERS_INSTALLATION_LOCATION ${PROJECT_NAME}/${LIB_NAME})

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
            # Set the installed headers location
            target_include_directories(${LIB_NAME}
                PUBLIC
                    $<BUILD_INTERFACE:${ABSOLUTE_HEADER_BASE_PATH}>
            )
        endif()

    endif()


    if(INSTALL_LIB)

        # Install library
        install(TARGETS ${LIB_NAME} EXPORT ${PROJECT_NAME}Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

        if(__INTERFACE)
        else()
            # Set the installed headers location
            target_include_directories(${LIB_NAME}
                PUBLIC
                    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${HEADERS_INSTALLATION_LOCATION}>
            )
        endif()

    endif()


endfunction()


# Export this package into CMake's global registry
# This allows users of our package to use the "find_package(PACKAGE_NAME)" statement
# to retrieve the definitions needed to build against our package
function(facelift_export_project)

    set(options)
    set(oneValueArgs )
    set(multiValueArgs BUILD_FILES INSTALLED_FILES)
    cmake_parse_arguments(ARGUMENT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

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



# Build and install a QML plugin
function(facelift_add_qml_plugin PLUGIN_NAME)

    set(options )
    set(oneValueArgs URI VERSION)
    set(multiValueArgs )
    cmake_parse_arguments(ARGUMENT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(URI ${ARGUMENT_URI})

    if(NOT ARGUMENT_VERSION)
        # Default to version 1.0 if no version is specified
        set(ARGUMENT_VERSION "1.0")
    endif()

    string(REPLACE "." ";" VERSION_LIST "${ARGUMENT_VERSION}")
    list(GET VERSION_LIST 0 PLUGIN_MAJOR_VERSION)
    list(GET VERSION_LIST 1 PLUGIN_MINOR_VERSION)

    string(REPLACE "." "/" PLUGIN_PATH ${URI})

    facelift_add_library(${PLUGIN_NAME} ${ARGUMENT_UNPARSED_ARGUMENTS} NO_INSTALL NO_EXPORT LINK_LIBRARIES Qt5::Qml)

    set_target_properties(${PLUGIN_NAME} PROPERTIES
        COMPILE_DEFINITIONS "PLUGIN_MINOR_VERSION=${PLUGIN_MINOR_VERSION};PLUGIN_MAJOR_VERSION=${PLUGIN_MAJOR_VERSION}"
    )

    set(INSTALL_PATH imports/${PLUGIN_PATH})

    set_target_properties(${PLUGIN_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${INSTALL_PATH}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${INSTALL_PATH}
    )

    install(TARGETS ${PLUGIN_NAME} DESTINATION ${INSTALL_PATH})

    install(FILES ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/qmldir DESTINATION ${INSTALL_PATH})
    file(WRITE ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/qmldir "module ${URI}\nplugin ${PLUGIN_NAME}\ntypeinfo plugins.qmltypes")

    if(NOT CMAKE_CROSSCOMPILING AND NOT WIN32)
        # not supported for now on Win32 since the required libraries can't be loaded without setting the PATH variable
        add_custom_command(
            OUTPUT  ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/plugins.qmltypes
            COMMAND ${_qt5Core_install_prefix}/bin/qmlplugindump -noinstantiate ${URI} ${PLUGIN_MAJOR_VERSION}.${PLUGIN_MINOR_VERSION} ${CMAKE_BINARY_DIR}/imports -output ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/plugins.qmltypes
            DEPENDS ${PLUGIN_NAME}
        )
        add_custom_target("generate_qmltypes_${PLUGIN_NAME}" ALL DEPENDS ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/plugins.qmltypes)
        install(FILES ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/plugins.qmltypes DESTINATION ${INSTALL_PATH})
    endif()

endfunction()
