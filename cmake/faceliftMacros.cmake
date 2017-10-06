
include(GNUInstallDirs)    # for standard installation locations
include(CMakePackageConfigHelpers)

set(FACELIFT_ENABLE_IPC ON)    # Force IPC for now

function(facelift_add_unity_files VAR_NAME FILE_LIST_)
    set(FILE_INDEX "0")

    unset(FILE_LIST)
    foreach(FILE ${FILE_LIST_})
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


function(facelift_add_package TARGET_NAME QFACE_MODULE_NAME INTERFACE_FOLDER)

    facelift_load_variables()
    facelift_module_to_libname(LIBRARY_NAME ${QFACE_MODULE_NAME})

    set(GENERATED_HEADERS_INSTALLATION_LOCATION ${FACELIFT_GENERATED_HEADERS_INSTALLATION_LOCATION}/${LIBRARY_NAME})

    get_property(CODEGEN_LOCATION GLOBAL PROPERTY FACELIFT_CODEGEN_LOCATION)
    set(QFACE_BASE_LOCATION ${CODEGEN_LOCATION}/facelift/qface)
    set(CODEGEN_EXECUTABLE_LOCATION ${CODEGEN_LOCATION}/facelift-codegen.py)

    message("QFace location : ${QFACE_BASE_LOCATION}")
    message("Facelift code generator executable : ${CODEGEN_EXECUTABLE_LOCATION}")

    set(ENV{PYTHONPATH} "ENV{PYTHONPATH}:${QFACE_BASE_LOCATION}")

    set(WORK_PATH ${CMAKE_CURRENT_BINARY_DIR}/facelift_generated_tmp)

    set(OUTPUT_PATH ${CMAKE_BINARY_DIR}/facelift_generated/${LIBRARY_NAME})  # Keep generated file folder outside of CMAKE_CURRENT_BINARY_DIR to avoid having the MOC generated file inside the same folder, which would cause unnecessary recompiles
    set(API_OUTPUT_PATH ${OUTPUT_PATH}/api)
    set(DUMMY_OUTPUT_PATH ${OUTPUT_PATH}/dummy)
    set(IPC_OUTPUT_PATH ${OUTPUT_PATH}/ipc)

    file(MAKE_DIRECTORY ${WORK_PATH})

    set(CODEGEN_COMMAND ${CODEGEN_EXECUTABLE_LOCATION} ${INTERFACE_FOLDER} ${WORK_PATH})

    execute_process(COMMAND ${CODEGEN_COMMAND}
        RESULT_VARIABLE CODEGEN_RETURN_CODE
        WORKING_DIRECTORY ${QFACE_BASE_LOCATION}/qface
    )

    if(NOT "${CODEGEN_RETURN_CODE}" STREQUAL "0")
        message(FATAL_ERROR "Facelift code generation failed. Command \"${CODEGEN_COMMAND}\". Return code: ${CODEGEN_RETURN_CODE}\n")
    endif()

    facelift_synchronize_folders(${WORK_PATH} ${OUTPUT_PATH})

    # Delete work folder
    file(REMOVE_RECURSE ${WORK_PATH})

    # Get the list of generated files
    facelift_add_library(${LIBRARY_NAME}_api 
        SOURCES_GLOB_RECURSE ${API_OUTPUT_PATH}/*.cpp
        HEADERS_GLOB_RECURSE ${API_OUTPUT_PATH}/*.h
        LINK_LIBRARIES ModelLib QMLModelLib PropertyLib
        PUBLIC_HEADER_BASE_PATH ${API_OUTPUT_PATH}
        UNITY_BUILD
    )

    facelift_add_library(${LIBRARY_NAME}_dummy
        SOURCES_GLOB_RECURSE ${DUMMY_OUTPUT_PATH}/*.cpp
        HEADERS_GLOB_RECURSE ${DUMMY_OUTPUT_PATH}/*.h
        LINK_LIBRARIES ${LIBRARY_NAME}_api DummyModelLib
        PUBLIC_HEADER_BASE_PATH ${DUMMY_OUTPUT_PATH}
        UNITY_BUILD
    )

    set(GENERATED_LIBRARIES ${LIBRARY_NAME}_api ${LIBRARY_NAME}_dummy)

	if(FACELIFT_ENABLE_IPC)

        facelift_add_library(${LIBRARY_NAME}_ipc
            SOURCES_GLOB_RECURSE ${IPC_OUTPUT_PATH}/*.cpp
            HEADERS_GLOB_RECURSE ${IPC_OUTPUT_PATH}/*.h
            LINK_LIBRARIES ${LIBRARY_NAME}_api IPCLib
            PUBLIC_HEADER_BASE_PATH ${IPC_OUTPUT_PATH}
            UNITY_BUILD
        )

    	set(GENERATED_LIBRARIES ${GENERATED_LIBRARIES} ${LIBRARY_NAME}_ipc)

	endif()

    add_library(${TARGET_NAME} INTERFACE)
    target_link_libraries(${TARGET_NAME} INTERFACE ${GENERATED_LIBRARIES})
    install(TARGETS ${TARGET_NAME} EXPORT ${PROJECT_NAME}Targets)

    # Add a dummy target to make the QFace files visible in the IDE
    file(GLOB_RECURSE QFACE_FILES ${INTERFACE_FOLDER}/*.qface)
    add_custom_target(FaceliftPackage_${LIBRARY_NAME} SOURCES ${QFACE_FILES})

endfunction()


#function(facelift_set_qml_implementation_path LIBRARY_NAME PATH)
# TODO : fix the way QML implementations are located
#    set_target_properties(${LIBRARY_NAME}_api PROPERTIES
#        COMPILE_DEFINITIONS "QML_MODEL_LOCATION=${PATH}"
#    )
#endfunction()


function(facelift_add_library LIB_NAME)

    set(options OPTIONAL NO_INSTALL UNITY_BUILD)
    set(oneValueArgs )
    set(multiValueArgs HEADERS HEADERS_GLOB_RECURSE SOURCES SOURCES_GLOB_RECURSE PUBLIC_HEADER_BASE_PATH LINK_LIBRARIES UI_FILES)
    cmake_parse_arguments(ARGUMENTS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(SOURCES ${ARGUMENTS_SOURCES})
    foreach(SOURCE_GLOB ${ARGUMENTS_SOURCES_GLOB_RECURSE})
        file(GLOB_RECURSE GLOB_FILES ${SOURCE_GLOB})
        set(SOURCES ${SOURCES} ${GLOB_FILES})
    endforeach()

    set(HEADERS ${ARGUMENTS_HEADERS})
    foreach(HEADER_GLOB ${ARGUMENTS_HEADERS_GLOB_RECURSE})
        file(GLOB_RECURSE GLOB_FILES ${HEADER_GLOB})
        set(HEADERS ${HEADERS} ${GLOB_FILES})
    endforeach()

    qt5_wrap_cpp(HEADERS_MOCS ${HEADERS} TARGET ${LIB_NAME})

    if(ARGUMENTS_UI_FILES)
        qt5_wrap_ui(UI_FILES ${ARGUMENTS_UI_FILES})
    endif()

    set(ALL_SOURCES ${SOURCES} ${HEADERS_MOCS} ${UI_FILES})

    set(UNITY_BUILD ${ARGUMENTS_UNITY_BUILD})
    if(${AUTO_UNITY_BUILD})
        set(UNITY_BUILD ON)
    endif()

    if(UNITY_BUILD)
        facelift_add_unity_files(ALL_SOURCES "${ALL_SOURCES}")
    endif()

    add_library(${LIB_NAME} SHARED ${ALL_SOURCES})

    # We assume every lib links against QtCore at least
    target_link_libraries(${LIB_NAME} Qt5::Core ${ARGUMENTS_LINK_LIBRARIES})

    if (NOT ${ARGUMENTS_NO_INSTALL})
        set(INSTALL_LIB ON)
    endif()

    if(NOT ARGUMENTS_PUBLIC_HEADER_BASE_PATH)
        set(PUBLIC_HEADER_BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
    else()
        set(PUBLIC_HEADER_BASE_PATH ${ARGUMENTS_PUBLIC_HEADER_BASE_PATH})
    endif()

    if (INSTALL_LIB)

        set(HEADERS_INSTALLATION_LOCATION ${PROJECT_NAME}/${LIB_NAME})

        get_filename_component(ABSOLUTE_HEADER_BASE_PATH "${PUBLIC_HEADER_BASE_PATH}" ABSOLUTE)

        # Create the directory for the installed headers
        install(DIRECTORY DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${HEADERS_INSTALLATION_LOCATION})

        # Install every headers
        foreach(HEADER ${HEADERS})
            get_filename_component(ABSOLUTE_HEADER_PATH ${HEADER} ABSOLUTE)
            file(RELATIVE_PATH relativePath "${PUBLIC_HEADER_BASE_PATH}" ${ABSOLUTE_HEADER_PATH})
            get_filename_component(ABSOLUTE_HEADER_INSTALLATION_DIR ${CMAKE_INSTALL_INCLUDEDIR}/${HEADERS_INSTALLATION_LOCATION}/${relativePath} DIRECTORY)
            install(FILES ${HEADER} DESTINATION ${ABSOLUTE_HEADER_INSTALLATION_DIR})
        endforeach()

        # Install library
        install(TARGETS ${LIB_NAME} EXPORT ${PROJECT_NAME}Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

        # Set the installed headers location
        target_include_directories(${LIB_NAME}
            PUBLIC
                $<BUILD_INTERFACE:${ABSOLUTE_HEADER_BASE_PATH}>
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${HEADERS_INSTALLATION_LOCATION}>
        )

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

    set(CONFIG_DESTINATION_PATH "${PROJECT_BINARY_DIR}")
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
