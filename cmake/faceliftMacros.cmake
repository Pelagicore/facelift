
option(IGNORE_AUTO_UNITY_BUILD "Disable unity build even if AUTO_UNITY_BUILD option is ON" OFF)
option(DISABLE_UNITY_BUILD "Completely disable unity build" OFF)
option(ENABLE_LTO "Enables Link Time Optimization" OFF)
option(DISABLE_DEVELOPMENT_FILE_INSTALLATION "Disable development file installation" OFF)
option(ENABLE_AUTO_MONOLITHIC "Enable monolithic support for all shared libraries" OFF)
option(ENABLE_MONOLITHIC_BUILD "Enable monolithic build" ${ENABLE_AUTO_MONOLITHIC})

if(ENABLE_MONOLITHIC_BUILD)
    if(${CMAKE_VERSION} VERSION_LESS "3.12.0")
        message(WARNING "CMake > 3.12.0 is required to enable monolithic build, but your version is ${CMAKE_VERSION}. Forcing ENABLE_MONOLITHIC_BUILD to OFF")
        set(ENABLE_MONOLITHIC_BUILD OFF)
    endif()
    cmake_policy(SET CMP0079 NEW)  # Needed to alter library definition in parent scope
endif()

include(GNUInstallDirs)    # for standard installation locations
include(CMakePackageConfigHelpers)

if(ENABLE_LTO)
    cmake_minimum_required(VERSION 3.9.0)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT IPO_AVAILABLE)
    if (IPO_AVAILABLE)
        message("Using LTO linker")
    else()
        message(WARNING "Link Time Optimization is requested but does not seem to be available on this build toolchain. The build might fail !")
    endif()
endif()

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

    set(INTERNAL_SEPARATOR "#")
    set(AGGREGATED_UNIT_FILE_LIST "")
    set(COMPLETE_FILE_LIST "")

    if(NOT "${FACELIFT_CACHED_LIST_${TARGET_NAME}}" STREQUAL "${FILE_LIST}")
        unset(UNITY_GENERATOR_INPUTS)
        foreach(file_path ${FILE_LIST})
            string(APPEND UNITY_GENERATOR_INPUTS "${file_path}\n")
        endForeach()
        set(UNITY_GENERATOR_FILE_LIST "${CMAKE_CURRENT_BINARY_DIR}/unityGeneratorFileNames_${TARGET_NAME}.txt")
        file(WRITE ${UNITY_GENERATOR_FILE_LIST} ${UNITY_GENERATOR_INPUTS})
        get_target_property(UNITY_LOCATION unity_generator LOCATION)
        set(UNITY_GENERATOR_COMMAND ${FACELIFT_PYTHON_EXECUTABLE} ${UNITY_LOCATION} --output ${CMAKE_CURRENT_BINARY_DIR} --target_name ${TARGET_NAME} --file_list ${UNITY_GENERATOR_FILE_LIST})
        execute_process(COMMAND ${UNITY_GENERATOR_COMMAND}
                        OUTPUT_VARIABLE COMPLETE_FILE_LIST
                        ERROR_VARIABLE UNITY_GENERATOR_ERROR
                        RESULT_VARIABLE UNITY_GENERATOR_RETURN_CODE)
        if(NOT "${UNITY_GENERATOR_RETURN_CODE}" STREQUAL "0")
            string(REPLACE ";" " " UNITY_GENERATOR_COMMAND "${UNITY_GENERATOR_COMMAND}")
            message(FATAL_ERROR "Facelift unity files generation failed!\n} with PYTHONPATH=$ENV{PYTHONPATH}\nCommand:${UNITY_GENERATOR_COMMAND}\nReturn code: ${UNITY_GENERATOR_RETURN_CODE}\nOutput: ${COMPLETE_FILE_LIST}\nError: ${UNITY_GENERATOR_ERROR}\n")
        endif()
        set(FACELIFT_CACHED_COMPLETE_FILE_LIST_${TARGET_NAME} "${COMPLETE_FILE_LIST}" CACHE INTERNAL "Store files")
    else()
        set(COMPLETE_FILE_LIST ${FACELIFT_CACHED_COMPLETE_FILE_LIST_${TARGET_NAME}})
    endif()

    foreach(SOURCES_IN_UNIT_LIST ${COMPLETE_FILE_LIST})
            string(REPLACE ${INTERNAL_SEPARATOR} ";" SOURCES_IN_UNIT_LIST ${SOURCES_IN_UNIT_LIST})

            # first file is unit file path
            list(GET SOURCES_IN_UNIT_LIST 0 UNIT_PATH)
            list(REMOVE_AT SOURCES_IN_UNIT_LIST 0)

            set_source_files_properties(${UNIT_PATH} PROPERTIES OBJECT_DEPENDS "${SOURCES_IN_UNIT_LIST}")
            list(APPEND AGGREGATED_UNIT_FILE_LIST ${UNIT_PATH})
    endforeach()

    set(FACELIFT_CACHED_LIST_${TARGET_NAME} "${FILE_LIST}" CACHE INTERNAL "stored file list for the target")

    set(${VAR_NAME} ${AGGREGATED_UNIT_FILE_LIST} ${NON_UNITY_FILE_LIST} PARENT_SCOPE)

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

    # Call the code generator if either the interface or the code generator has changed
    get_target_property(CODEGEN_EXECUTABLE_LOCATION facelift-codegen LOCATION)
    get_filename_component(CODEGEN_LOCATION ${CODEGEN_EXECUTABLE_LOCATION} DIRECTORY)
    set(QFACE_BASE_LOCATION ${CODEGEN_LOCATION}/qface)
    file(GLOB_RECURSE QFACE_FILES ${ARGUMENT_INTERFACE_DEFINITION_FOLDER}/*.qface)
    file(GLOB_RECURSE CODEGEN_FILES ${CODEGEN_LOCATION}/*)

    unset(MODIFIED_FILES)
    foreach(FILE ${QFACE_FILES} ${CODEGEN_FILES})
        file(TIMESTAMP ${FILE} MOD_TIME "%s")
        if(MOD_TIME GREATER FACELIFT_LAST_BUILD_TIMESTAMP_${LIBRARY_NAME})
            list(APPEND MODIFIED_FILES ${FILE})
        endif()
    endforeach()

    # needed if qface file is removed
    set(FORCE_GENERATION NO)
    list(LENGTH QFACE_FILES NUM_QFACE_FILES)
    if(NOT NUM_QFACE_FILES_${LIBRARY_NAME} EQUAL NUM_QFACE_FILES)
        set(FORCE_GENERATION YES)
    endif()

    # force generation if not run previously
    if(NOT FACELIFT_LAST_BUILD_TIMESTAMP_${LIBRARY_NAME})
        set(FORCE_GENERATION YES)
    endif()

    set(NUM_QFACE_FILES_${LIBRARY_NAME} ${NUM_QFACE_FILES} CACHE INTERNAL "Number of qface files for library")

    list(LENGTH MODIFIED_FILES NUM_MODIFIED_FILES)
    if(NUM_MODIFIED_FILES OR FORCE_GENERATION)
        file(TO_NATIVE_PATH "${QFACE_BASE_LOCATION}" QFACE_BASE_LOCATION_NATIVE_PATH)
        set(ENV{PYTHONPATH} "${QFACE_BASE_LOCATION_NATIVE_PATH}")

        set(WORK_PATH ${CMAKE_CURRENT_BINARY_DIR}/facelift_generated_tmp)
        file(MAKE_DIRECTORY ${WORK_PATH})

        set(BASE_CODEGEN_COMMAND ${CODEGEN_EXECUTABLE_LOCATION} --input "${ARGUMENT_INTERFACE_DEFINITION_FOLDER}" --output "${WORK_PATH}")

        foreach(IMPORT_FOLDER ${ARGUMENT_IMPORT_FOLDERS})
            list(APPEND BASE_CODEGEN_COMMAND "--dependency" "${IMPORT_FOLDER}")
        endforeach()

        if(ARGUMENT_GENERATE_ALL)
            list(APPEND BASE_CODEGEN_COMMAND "--all")
        endif()

        list(APPEND BASE_CODEGEN_COMMAND "--no_moc_file_path" "${ARGUMENT_OUTPUT_PATH}")

        string(REPLACE ";" " " BASE_CODEGEN_COMMAND_WITH_SPACES "${BASE_CODEGEN_COMMAND}")
        message("Calling facelift code generator. Command:\n PYTHONPATH=$ENV{PYTHONPATH} ${FACELIFT_PYTHON_EXECUTABLE} ${BASE_CODEGEN_COMMAND_WITH_SPACES}")

        if(NOT DEFINED ENV{LANG}) # e.g. Qt Creator was started from the Apple Dock
            set(ENV{LANG} "en_US.UTF-8")
            # --> http://click.pocoo.org/5/python3/#python-3-surrogate-handling
            # --> https://apple.stackexchange.com/questions/54765/how-can-i-have-qt-creator-to-recognize-my-environment-variables
            set(LANG_SET_BY_FACELIFT true)
        else()
            set(LANG_SET_BY_FACELIFT false)
        endif()

        execute_process(COMMAND ${FACELIFT_PYTHON_EXECUTABLE} ${BASE_CODEGEN_COMMAND}
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
    endif()

    # Add a dependency so that CMake will reconfigure whenever one of the interface files is changed, which will refresh our generated files
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CODEGEN_FILES};${QFACE_FILES};${ARGUMENT_INTERFACE_DEFINITION_FOLDER}/")

    string(TIMESTAMP FACELIFT_BUILD_TIMESTAMP "%s")
    set(FACELIFT_LAST_BUILD_TIMESTAMP_${LIBRARY_NAME} ${FACELIFT_BUILD_TIMESTAMP} CACHE INTERNAL "timestamp of last build")

endfunction()


function(facelift_add_interface TARGET_NAME)

    set(options GENERATE_ALL MONOLITHIC_SUPPORTED)
    set(oneValueArgs INTERFACE_DEFINITION_FOLDER )
    set(multiValueArgs IMPORT_FOLDERS LINK_LIBRARIES MONOLITHIC_LINK_LIBRARIES)
    cmake_parse_arguments(ARGUMENT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    facelift_load_variables()
    set(LIBRARY_NAME ${TARGET_NAME})

    set(GENERATED_HEADERS_INSTALLATION_LOCATION ${FACELIFT_GENERATED_HEADERS_INSTALLATION_LOCATION}/${LIBRARY_NAME})

    set(OUTPUT_PATH ${CMAKE_BINARY_DIR}/facelift_generated/${LIBRARY_NAME})  # Keep generated file folder outside of CMAKE_CURRENT_BINARY_DIR to avoid having the MOC generated file inside the same folder, which would cause unnecessary recompiles


    set(TYPES_OUTPUT_PATH ${OUTPUT_PATH}/types)
    set(DEVTOOLS_OUTPUT_PATH ${OUTPUT_PATH}/devtools)
    set(IPC_OUTPUT_PATH ${OUTPUT_PATH}/ipc)
    set(IPC_DBUS_OUTPUT_PATH ${OUTPUT_PATH}/ipc_dbus)
    set(MODULE_OUTPUT_PATH ${OUTPUT_PATH}/module)

    unset(ADDITIONAL_ARGUMENTS)
    if(ARGUMENT_GENERATE_ALL)
        set(ADDITIONAL_ARGUMENTS "GENERATE_ALL")
    endif()

    facelift_generate_code(INTERFACE_DEFINITION_FOLDER ${ARGUMENT_INTERFACE_DEFINITION_FOLDER} IMPORT_FOLDERS ${ARGUMENT_IMPORT_FOLDERS} OUTPUT_PATH ${OUTPUT_PATH} LIBRARY_NAME ${TARGET_NAME} ${ADDITIONAL_ARGUMENTS})

    unset(ARGS)

    # Get the list of generated files
    list(APPEND ARGS
        SOURCES_GLOB_RECURSE ${TYPES_OUTPUT_PATH}/*.cpp
        HEADERS_GLOB_RECURSE ${TYPES_OUTPUT_PATH}/*.h
        PUBLIC_HEADER_BASE_PATH ${TYPES_OUTPUT_PATH}
    )

    if(TARGET FaceliftIPCLib)

        if(TARGET FaceliftIPCLibDBus)
            list(APPEND ARGS
                SOURCES_GLOB_RECURSE ${IPC_DBUS_OUTPUT_PATH}/*.cpp
                HEADERS_GLOB_RECURSE_NO_INSTALL ${IPC_DBUS_OUTPUT_PATH}/*.h
                LINK_LIBRARIES FaceliftIPCLibDBus
            )
        endif()

        list(APPEND ARGS
            SOURCES_GLOB_RECURSE ${IPC_OUTPUT_PATH}/*.cpp
            HEADERS_GLOB_RECURSE ${IPC_OUTPUT_PATH}/*.h
            LINK_LIBRARIES FaceliftIPCLib
            PUBLIC_HEADER_BASE_PATH ${IPC_OUTPUT_PATH}
        )
        list(APPEND MODULE_COMPILE_DEFINITIONS ENABLE_IPC)

    endif()

    if(ARGUMENT_MONOLITHIC_SUPPORTED)
        set(ARGUMENT_MONOLITHIC MONOLITHIC_SUPPORTED)
    endif()

    if(TARGET FaceliftIPCLibDBus)
        set(PRIVATE_INCLUDE_DIRECTORIES ${IPC_DBUS_OUTPUT_PATH})
    endif()

    facelift_add_library(${LIBRARY_NAME}
        SOURCES_GLOB_RECURSE ${MODULE_OUTPUT_PATH}/*.cpp
        HEADERS_GLOB_RECURSE ${MODULE_OUTPUT_PATH}/*.h
        LINK_LIBRARIES FaceliftModelLib ${ARGUMENT_LINK_LIBRARIES}
        MONOLITHIC_LINK_LIBRARIES ${ARGUMENT_MONOLITHIC_LINK_LIBRARIES}
        PUBLIC_HEADER_BASE_PATH ${MODULE_OUTPUT_PATH}
        ${ARGS}
        UNITY_BUILD
        PRIVATE_DEFINITIONS
            ${MODULE_COMPILE_DEFINITIONS}
        ${ARGUMENT_MONOLITHIC}
        PRIVATE_INCLUDE_DIRECTORIES ${PRIVATE_INCLUDE_DIRECTORIES}
    )

    # Get the list of files from the interface definition folder so that they are shown in QtCreator
    file(GLOB_RECURSE QFACE_FILES ${ARGUMENT_INTERFACE_DEFINITION_FOLDER}/*.qface)
    get_target_property(target_type ${LIBRARY_NAME} TYPE)
    if(NOT target_type STREQUAL "INTERFACE_LIBRARY")
        target_sources(${LIBRARY_NAME} PRIVATE ${QFACE_FILES})
    endif()

endfunction()


#function(facelift_set_qml_implementation_path LIBRARY_NAME PATH)
# TODO : fix the way QML implementations are located
#    set_target_properties(${LIBRARY_NAME}_api PROPERTIES
#        COMPILE_DEFINITIONS "QML_MODEL_LOCATION=${PATH}"
#    )
#endfunction()


macro(_facelift_parse_target_arguments additionalOptions additionalOneValueArgs additionalMultiValueArgs)

    set(options NO_INSTALL UNITY_BUILD NO_EXPORT INTERFACE ${additionalOptions})
    set(oneValueArgs ${additionalOneValueArgs} HEADERS_NO_MOC_FILE)
    set(multiValueArgs PRIVATE_DEFINITIONS PUBLIC_DEFINITIONS PROPERTIES COMPILE_OPTIONS
        HEADERS HEADERS_GLOB HEADERS_GLOB_RECURSE
        HEADERS_NO_MOC
        SOURCES SOURCES_GLOB SOURCES_GLOB_RECURSE
        LINK_LIBRARIES
        UI_FILES
        RESOURCE_FOLDERS
        PRIVATE_INCLUDE_DIRECTORIES
        ${additionalMultiValueArgs}
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

    set(HEADERS_NO_MOC ${ARGUMENT_HEADERS_NO_MOC})

    set(HEADERS_NO_MOC_FILE ${ARGUMENT_HEADERS_NO_MOC_FILE})

    unset(HEADERS_GLOB)
    foreach(HEADER ${ARGUMENT_HEADERS_GLOB_RECURSE})
        file(GLOB_RECURSE GLOB_FILES ${HEADER})
        list(APPEND HEADERS_GLOB ${GLOB_FILES})
    endforeach()
    foreach(HEADER ${ARGUMENT_HEADERS_GLOB})
        file(GLOB GLOB_FILES ${HEADER})
        list(APPEND HEADERS_GLOB ${GLOB_FILES})
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

endmacro()

macro(_facelift_add_target_start IMPLEMENTATION_TARGET_NAME)

    if(ALIAS_NAME)
        set(NO_MOC_PATH ${CMAKE_BINARY_DIR}/facelift_generated/${ALIAS_NAME})
    else()
        set(NO_MOC_PATH ${CMAKE_BINARY_DIR}/facelift_generated/${TARGET_NAME})
    endif()

    # set HEADERS_NO_MOC_GENERATED in this include
    include("${NO_MOC_PATH}/no_moc.cmake" OPTIONAL)

    # set HEADERS_NO_MOC_FROM_FILE in this include (the file is the input parameter)
    if(HEADERS_NO_MOC_FILE)
        include("${HEADERS_NO_MOC_FILE}" OPTIONAL)
    endif()

    set(HEADERS_FOR_MOC ${HEADERS_GLOB} ${HEADERS_NO_INSTALL})
    
    foreach(HEADER ${HEADERS_NO_MOC_GENERATED} ${HEADERS_NO_MOC})
        list(REMOVE_ITEM HEADERS_FOR_MOC ${HEADER})
    endforeach()

    foreach(HEADER ${HEADERS_FOR_MOC})
        list(FIND HEADERS_NO_MOC_FROM_FILE ${HEADER} IS_FOUND)

        if(${IS_FOUND} GREATER_EQUAL 0)
            list(REMOVE_ITEM HEADERS_FOR_MOC ${HEADER})
        endif()

    endforeach()

    list(APPEND HEADERS_FOR_MOC ${HEADERS})

    unset(HEADERS_MOCS)

    if(HEADERS_FOR_MOC)
        qt5_wrap_cpp(HEADERS_MOCS ${HEADERS_FOR_MOC} TARGET ${IMPLEMENTATION_TARGET_NAME})
    endif()

    # add processed for moc headers to the list of headers for installation
    list(APPEND HEADERS ${HEADERS_GLOB} ${HEADERS_NO_MOC})

    unset(UI_FILES)
    if(ARGUMENT_UI_FILES)
        qt5_wrap_ui(UI_FILES ${ARGUMENT_UI_FILES})
    endif()

    set(ALL_SOURCES ${SOURCES} ${HEADERS_MOCS} ${UI_FILES})

    if(NOT DISABLE_UNITY_BUILD)
        if("${AUTO_UNITY_BUILD}" AND NOT "${IGNORE_AUTO_UNITY_BUILD}")
            set(UNITY_BUILD ON)
        else()
            set(UNITY_BUILD ${ARGUMENT_UNITY_BUILD})
        endif()
    endif()

    if(CMAKE_AUTOMOC)
        set(UNITY_BUILD OFF)
    endif()

    if(UNITY_BUILD)
        facelift_add_unity_files(${IMPLEMENTATION_TARGET_NAME} ALL_SOURCES "${ALL_SOURCES}")
    endif()

endmacro()

macro(_facelift_add_target_finish INTERFACE_TARGET_NAME IMPLEMENTATION_TARGET_NAME)

    get_target_property(target_type ${IMPLEMENTATION_TARGET_NAME} TYPE)
    if(NOT target_type STREQUAL "INTERFACE_LIBRARY")

        # create a valid preprocessor macro based on the target name
        string(REPLACE "-" "_" LIB_PREPROCESSOR_DEFINITION "${INTERFACE_TARGET_NAME}_LIBRARY")
        target_compile_definitions(${IMPLEMENTATION_TARGET_NAME} PRIVATE ${LIB_PREPROCESSOR_DEFINITION} ${ARGUMENT_PRIVATE_DEFINITIONS})

        target_compile_options(${IMPLEMENTATION_TARGET_NAME} PRIVATE ${ARGUMENT_COMPILE_OPTIONS})

        target_include_directories(${IMPLEMENTATION_TARGET_NAME} PRIVATE ${ARGUMENT_PRIVATE_INCLUDE_DIRECTORIES})

    endif()

    unset(INTERFACE_IF_INTERFACE_TARGET)
    get_target_property(target_type ${INTERFACE_TARGET_NAME} TYPE)
    if(target_type STREQUAL "INTERFACE_LIBRARY")
        set(INTERFACE_TARGET_MODIFIER INTERFACE)
        set(INTERFACE_IF_INTERFACE_TARGET INTERFACE)
    else()
        set(INTERFACE_TARGET_MODIFIER PUBLIC)
    endif()

    target_compile_definitions(${INTERFACE_TARGET_NAME} ${INTERFACE_TARGET_MODIFIER} ${ARGUMENT_PUBLIC_DEFINITIONS})

    if(ARGUMENT_PROPERTIES)
        set_target_properties(${IMPLEMENTATION_TARGET_NAME} PROPERTIES ${ARGUMENT_PROPERTIES})
    endif()

    # We assume every lib links against QtCore at least
    target_link_libraries(${INTERFACE_TARGET_NAME} ${INTERFACE_IF_INTERFACE_TARGET} Qt5::Core ${ARGUMENT_LINK_LIBRARIES})

endmacro()



function(facelift_add_library TARGET_NAME)

    _facelift_parse_target_arguments("SYSTEM;STATIC;SHARED;OBJECT;MODULE;MONOLITHIC_SUPPORTED;NO_MONOLITHIC" ""
        "HEADERS_NO_INSTALL;HEADERS_GLOB_NO_INSTALL;HEADERS_GLOB_RECURSE_NO_INSTALL;PUBLIC_HEADER_BASE_PATH;MONOLITHIC_LINK_LIBRARIES"
        ${ARGN}
    )

    if(ARGUMENT_SYSTEM)
        message("System library ${TARGET_NAME}")
        set(SYSTEM_LIB_ARGUMENT SYSTEM)
    else()
        unset(SYSTEM_LIB_ARGUMENT)
    endif()

    list(APPEND ARGUMENT_LINK_LIBRARIES ${ARGUMENT_MONOLITHIC_LINK_LIBRARIES})

    unset(__INTERFACE)
    if(ARGUMENT_INTERFACE)
        _facelift_add_target_start(${TARGET_NAME})
        add_library(${TARGET_NAME} INTERFACE)
        set(__INTERFACE INTERFACE)
        _facelift_add_target_finish(${TARGET_NAME} ${TARGET_NAME})
    else()
        if(${ARGUMENT_STATIC})
            set(LIBRARY_TYPE STATIC)
            set(PREVENT_MONOLITHIC ON)
        endif()
        if(${ARGUMENT_OBJECT})
            set(LIBRARY_TYPE OBJECT)
            set(PREVENT_MONOLITHIC ON)
        endif()
        if(${ARGUMENT_MODULE})
            set(LIBRARY_TYPE MODULE)
            set(PREVENT_MONOLITHIC ON)
        endif()
        if(${ARGUMENT_SHARED})
            set(LIBRARY_TYPE SHARED)
        endif()

        if(ARGUMENT_MONOLITHIC_SUPPORTED OR (ENABLE_AUTO_MONOLITHIC AND (NOT ARGUMENT_NO_MONOLITHIC) AND (NOT PREVENT_MONOLITHIC)))

            set(ALIAS_NAME ${TARGET_NAME})

            if(ENABLE_MONOLITHIC_BUILD)

                set_property(GLOBAL PROPERTY ${TARGET_NAME}_IS_MONOLITHIC ON)

                set(TARGET_NAME ${TARGET_NAME}_)

                set(__INTERFACE INTERFACE)

                add_library(${TARGET_NAME} INTERFACE)

                set(IMPLEMENTATION_TARGET_NAME ${TARGET_NAME}_OBJECTS)

                _facelift_add_target_start(${IMPLEMENTATION_TARGET_NAME})

                if(NOT ALL_SOURCES)
                    add_library(${IMPLEMENTATION_TARGET_NAME} INTERFACE)
                else()
                    add_library(${IMPLEMENTATION_TARGET_NAME} OBJECT ${ALL_SOURCES})
                    target_link_libraries(${IMPLEMENTATION_TARGET_NAME} ${TARGET_NAME})
                endif()

                set_property(GLOBAL APPEND PROPERTY ${ALIAS_NAME}_LINK_LIBRARIES ${ARGUMENT_LINK_LIBRARIES})
                get_property(LINK_LIBRARIES GLOBAL PROPERTY ${TARGET_NAME}_LINK_LIBRARIES)

                list(APPEND ARGUMENT_LINK_LIBRARIES ${MONOLITHIC_LINK_LIBRARIES})
                unset(ARGUMENT_LINK_LIBRARIES)
                _facelift_add_target_finish(${TARGET_NAME} ${IMPLEMENTATION_TARGET_NAME})

                # Append our object library to the list of libraries to be linked to the monolithic library
                set_property(GLOBAL APPEND PROPERTY MONOLITHIC_LIBRARIES ${ALIAS_NAME})

                # Create an interface library with the original target name, which links against the monolithic library
                add_library(${ALIAS_NAME} INTERFACE)
                target_link_libraries(${ALIAS_NAME} INTERFACE ${TARGET_NAME} ${CMAKE_PROJECT_NAME})

                set(BUILD_MONOLITHIC ON)

            else()

                set(TARGET_NAME ${TARGET_NAME}_OBJECTS)
                add_library(${ALIAS_NAME} INTERFACE)
                target_link_libraries(${ALIAS_NAME} INTERFACE ${TARGET_NAME})

                set(BUILD_MONOLITHIC OFF)

            endif()

            install(TARGETS ${ALIAS_NAME} EXPORT ${PROJECT_NAME}Targets)

        endif()

        if (NOT BUILD_MONOLITHIC)

            _facelift_add_target_start(${TARGET_NAME})
            if(NOT ALL_SOURCES)
                # We create an INTERFACE library if the library contains no source file
                set(__INTERFACE INTERFACE)
                set(LIBRARY_TYPE INTERFACE)
            endif()
            add_library(${TARGET_NAME} ${LIBRARY_TYPE} ${ALL_SOURCES})
            target_link_libraries(${TARGET_NAME} ${__INTERFACE} ${ARGUMENT_MONOLITHIC_LINK_LIBRARIES})
            _facelift_add_target_finish(${TARGET_NAME} ${TARGET_NAME})

            if(ENABLE_LTO)
               if(NOT "${LIBRARY_TYPE}" STREQUAL "INTERFACE")
                   set_property(TARGET ${TARGET_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
               endif()
            endif()
        endif()

    endif()

    if (NOT ${ARGUMENT_NO_INSTALL})
        set(INSTALL_LIB ON)
    endif()

    if (NOT ${ARGUMENT_NO_EXPORT})
        set(EXPORT_LIB ON)
    endif()

    if(NOT ARGUMENT_PUBLIC_HEADER_BASE_PATH)
        set(PUBLIC_HEADER_BASE_PATHS ${CMAKE_CURRENT_SOURCE_DIR})
    else()
        set(PUBLIC_HEADER_BASE_PATHS ${ARGUMENT_PUBLIC_HEADER_BASE_PATH})
    endif()

    if (EXPORT_LIB)

        set(HEADERS_INSTALLATION_LOCATION ${PROJECT_NAME}/${TARGET_NAME})

        if (NOT DISABLE_DEVELOPMENT_FILE_INSTALLATION)
            # Create the directory for the installed headers
            install(DIRECTORY DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${HEADERS_INSTALLATION_LOCATION})

            # Install headers
            foreach(HEADER ${HEADERS})

                set(isHeaderInstalled OFF)

                foreach(PUBLIC_HEADER_BASE_PATH ${PUBLIC_HEADER_BASE_PATHS})
                    get_filename_component(ABSOLUTE_HEADER_PATH ${HEADER} ABSOLUTE)
                    file(RELATIVE_PATH relativePath "${PUBLIC_HEADER_BASE_PATH}" ${ABSOLUTE_HEADER_PATH})
                    string(SUBSTRING "${relativePath}" 0 1 relativePathStart)
                    if(NOT "${relativePathStart}" STREQUAL ".")
                        get_filename_component(ABSOLUTE_HEADER_INSTALLATION_DIR ${CMAKE_INSTALL_INCLUDEDIR}/${HEADERS_INSTALLATION_LOCATION}/${relativePath} DIRECTORY)
                        install(FILES ${HEADER} DESTINATION ${ABSOLUTE_HEADER_INSTALLATION_DIR})
                        set(isHeaderInstalled ON)
                    endif()
                endforeach()

                if (NOT isHeaderInstalled)
                    message(FATAL_ERROR "Header '${HEADER}' ('${ABSOLUTE_HEADER_PATH}') is outside of public header paths:('${PUBLIC_HEADER_BASE_PATHS}')")
                endif()

            endforeach()
        endif()

        if(__INTERFACE)
            foreach(PUBLIC_HEADER_BASE_PATH ${PUBLIC_HEADER_BASE_PATHS})
                get_filename_component(ABSOLUTE_HEADER_BASE_PATH "${PUBLIC_HEADER_BASE_PATH}" ABSOLUTE)

                # Set the installed header location
                target_include_directories(${TARGET_NAME} ${SYSTEM_LIB_ARGUMENT}
                    INTERFACE
                        $<BUILD_INTERFACE:${ABSOLUTE_HEADER_BASE_PATH}>
                )
            endforeach()
        else()
            # Do not define target include directories if no headers are present. This avoids the creation and inclusion of empty directories.
            if(HEADERS)

                foreach(PUBLIC_HEADER_BASE_PATH ${PUBLIC_HEADER_BASE_PATHS})
                    get_filename_component(ABSOLUTE_HEADER_BASE_PATH "${PUBLIC_HEADER_BASE_PATH}" ABSOLUTE)

                    # Set the installed header location
                    target_include_directories(${TARGET_NAME} ${SYSTEM_LIB_ARGUMENT}
                        PUBLIC
                            $<BUILD_INTERFACE:${ABSOLUTE_HEADER_BASE_PATH}>
                    )
                endforeach()
            endif()
        endif()

        set(EXPORT_ARGUMENTS EXPORT ${PROJECT_NAME}Targets)

    else()

        unset(EXPORT_ARGUMENTS)

    endif()

    if(INSTALL_LIB)

        # Install library
        install(TARGETS ${TARGET_NAME} ${EXPORT_ARGUMENTS} DESTINATION ${CMAKE_INSTALL_LIBDIR})

        # Do not define target include directories if no headers are present. This avoids the creation and inclusion of empty directories.
        if(HEADERS)
            # Set the installed headers location
            target_include_directories(${TARGET_NAME} ${SYSTEM_LIB_ARGUMENT}
                INTERFACE
                    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${HEADERS_INSTALLATION_LOCATION}>
            )
        endif()

    endif()

endfunction()


function(facelift_add_executable TARGET_NAME)
    _facelift_parse_target_arguments("" "" "" ${ARGN})
    _facelift_add_target_start(${TARGET_NAME})
    add_executable(${TARGET_NAME} ${ALL_SOURCES})

    if (NOT ${ARGUMENT_NO_INSTALL})
        install(TARGETS ${TARGET_NAME} EXPORT ${PROJECT_NAME}Targets DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()

    _facelift_add_target_finish(${TARGET_NAME} ${TARGET_NAME})
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

    facelift_export_monolithic()

endfunction()


function(facelift_export_monolithic)

    if (${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})  # We only export the main project
        get_property(MONOLITHIC_LIBRARIES GLOBAL PROPERTY MONOLITHIC_LIBRARIES)
        if(MONOLITHIC_LIBRARIES AND NOT TARGET ${CMAKE_PROJECT_NAME})

            add_library(${CMAKE_PROJECT_NAME} SHARED)

            foreach(LIB ${MONOLITHIC_LIBRARIES})
                target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${LIB}__OBJECTS)

                get_property(LINK_LIBRARIES GLOBAL PROPERTY ${LIB}_LINK_LIBRARIES)

                foreach(LINK_LIBRARY ${LINK_LIBRARIES})

                    get_property(IS_MONOLITHIC GLOBAL PROPERTY ${LINK_LIBRARY}_IS_MONOLITHIC SET)

                    if (IS_MONOLITHIC)
                        target_link_libraries(${LIB} INTERFACE ${LINK_LIBRARY})
                        target_link_libraries(${LIB}_ INTERFACE ${LINK_LIBRARY}_)
                    else()
                        target_link_libraries(${LIB}_ INTERFACE ${LINK_LIBRARY})
                    endif()

                endforeach()

            endforeach()
            install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})
        endif()
    endif()

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

    facelift_add_library(${PLUGIN_NAME}
        ${ARGUMENT_UNPARSED_ARGUMENTS}
        NO_INSTALL
        NO_EXPORT
        MODULE
        LINK_LIBRARIES
            Qt5::Qml
        PRIVATE_DEFINITIONS
            -DPLUGIN_MINOR_VERSION=${PLUGIN_MINOR_VERSION}
            -DPLUGIN_MAJOR_VERSION=${PLUGIN_MAJOR_VERSION}
        PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${INSTALL_PATH}
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${INSTALL_PATH}
    )

    get_target_property(TARGET_TYPE ${PLUGIN_NAME} TYPE)
    if(TARGET_TYPE STREQUAL "STATIC_LIBRARY")
        target_compile_definitions(${PLUGIN_NAME} PRIVATE QT_STATICPLUGIN)
    endif()

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

    if(NOT CMAKE_CROSSCOMPILING)
        add_custom_command(
            OUTPUT  ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/plugins.qmltypes
            COMMAND ${_qt5Core_install_prefix}/bin/qmlplugindump -noinstantiate ${URI} ${PLUGIN_MAJOR_VERSION}.${PLUGIN_MINOR_VERSION} ${CMAKE_BINARY_DIR}/imports -output ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/plugins.qmltypes || touch ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/plugins.qmltypes
            DEPENDS ${PLUGIN_NAME}
        )
        add_custom_target("generate_qmltypes_${PLUGIN_NAME}" ALL DEPENDS ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/plugins.qmltypes)
        install(FILES ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/plugins.qmltypes DESTINATION ${INSTALL_PATH})
    endif()

endfunction()
