
include(GNUInstallDirs)    # for standard installation locations

get_property(CODEGEN_LOCATION_DEFINED GLOBAL PROPERTY FACELIFT_CODEGEN_LOCATION)

if(CODEGEN_LOCATION_DEFINED)
else()
    message("NOT DEFINED")
    set_property(GLOBAL PROPERTY FACELIFT_CODEGEN_LOCATION ${CMAKE_CURRENT_LIST_DIR}/${CODEGEN_RELATIVE_PATH})
endif()

get_property(CODEGEN_LOCATION GLOBAL PROPERTY FACELIFT_CODEGEN_LOCATION)
message("CODEGEN_LOCATION : ${CODEGEN_LOCATION}")


function(facelift_check_return_code ERROR_CODE)
    if(NOT "${ERROR_CODE}" STREQUAL "0")
        message(FATAL_ERROR "Failed")
    endif()
endfunction()


function(facelift_add_aggregator_library LIB_NAME FILE_LIST_)
    set(FILE_INDEX "0")
    set(FILE_LIST ${FILE_LIST_})
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
        set(FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}_aggregated_${FILE_INDEX}.cpp)
        foreach(SRC_FILE ${FILES})
            set(FILE_CONTENT "${FILE_CONTENT}#include \"${SRC_FILE}\"\n")
        endforeach()

        # To avoid unnecessary recompiles, check if it is really necessary to rewrite the aggregator file
        if(EXISTS ${FILE_NAME})
            file(READ ${FILE_NAME} OLD_FILE_CONTENT)
        endif()

        if(NOT "${OLD_FILE_CONTENT}" STREQUAL "${FILE_CONTENT}")
            file(WRITE ${FILE_NAME} ${FILE_CONTENT})
        endif()

        list(APPEND AGGREGATED_FILE_LIST ${FILE_NAME})

    endwhile()

    add_library(${LIB_NAME} SHARED ${AGGREGATED_FILE_LIST})

    # Add dummy target to ensure that MOC files are generated before building the library
    add_custom_target(${LIB_NAME}_dummyTarget DEPENDS ${FILE_LIST_})
    add_dependencies(${LIB_NAME} ${LIB_NAME}_dummyTarget)

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

    # Remove the files from DESTINATION, which can not be found in SOURCE
    file(GLOB_RECURSE DESTINATION_FILES RELATIVE ${FOLDER_DESTINATION} ${FOLDER_DESTINATION}/*)
    foreach(FILE_PATH ${DESTINATION_FILES})
        if(NOT EXISTS "${FOLDER_SOURCE}/${FILE_PATH}")
            file(REMOVE "${FOLDER_DESTINATION}/${FILE_PATH}")
        endif()
    endforeach()

endfunction()


function(facelift_add_package LIBRARY_NAME INTERFACE_FOLDER)

    get_property(CODEGEN_LOCATION GLOBAL PROPERTY FACELIFT_CODEGEN_LOCATION)
    set(QFACE_BASE_LOCATION ${CODEGEN_LOCATION}/facelift/qface)

    message("QFace location : ${QFACE_BASE_LOCATION}")
    message("Facelift code generator executable : ${CODEGEN_LOCATION}")

    set(ENV{PYTHONPATH} "ENV{PYTHONPATH}:${QFACE_BASE_LOCATION}")

    set(WORK_PATH ${CMAKE_CURRENT_BINARY_DIR}/facelift_generated_tmp)

    set(OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/facelift_generated/${LIBRARY_NAME})
    set(API_OUTPUT_PATH ${OUTPUT_PATH}/api)
    set(DUMMY_OUTPUT_PATH ${OUTPUT_PATH}/dummy)
    set(IPC_OUTPUT_PATH ${OUTPUT_PATH}/ipc)

    include_directories(${API_OUTPUT_PATH})

    file(MAKE_DIRECTORY ${WORK_PATH})

    execute_process(COMMAND ${CODEGEN_LOCATION}/facelift-codegen.py
        ${INTERFACE_FOLDER} ${WORK_PATH}
        RESULT_VARIABLE CODEGEN_RETURN_CODE
        WORKING_DIRECTORY ${QFACE_BASE_LOCATION}/qface
    )
    facelift_check_return_code(${CODEGEN_RETURN_CODE})

    facelift_synchronize_folders(${WORK_PATH} ${OUTPUT_PATH})

    # Delete work folder
    file(REMOVE_RECURSE ${WORK_PATH})

    # Get the list of generated files
    file(GLOB_RECURSE GENERATED_FILES ${API_OUTPUT_PATH}/*.*)
    file(GLOB_RECURSE GENERATED_FILES_HEADERS ${API_OUTPUT_PATH}/*.h)
    qt5_wrap_cpp(API_GENERATED_FILES_HEADERS_MOCS ${GENERATED_FILES_HEADERS} TARGET ${LIBRARY_NAME}_api)
    facelift_add_aggregator_library(${LIBRARY_NAME}_api "${GENERATED_FILES};${GENERATED_FILES_HEADERS};${API_GENERATED_FILES_HEADERS_MOCS}")
    target_link_libraries(${LIBRARY_NAME}_api ModelLib QMLModelLib PropertyLib)
    target_include_directories(${LIBRARY_NAME}_api PUBLIC ${API_OUTPUT_PATH})

    file(GLOB_RECURSE DUMMY_GENERATED_FILES ${DUMMY_OUTPUT_PATH}/*.*)
    file(GLOB_RECURSE DUMMY_GENERATED_FILES_HEADERS ${DUMMY_OUTPUT_PATH}/*.h)
    qt5_wrap_cpp(DUMMY_GENERATED_FILES_HEADERS_MOCS ${DUMMY_GENERATED_FILES_HEADERS} TARGET ${LIBRARY_NAME}_dummy)
    facelift_add_aggregator_library(${LIBRARY_NAME}_dummy "${DUMMY_GENERATED_FILES};${DUMMY_GENERATED_FILES_HEADERS};${DUMMY_GENERATED_FILES_HEADERS_MOCS}")
    target_link_libraries(${LIBRARY_NAME}_dummy ${LIBRARY_NAME}_api DummyModelLib)
    target_include_directories(${LIBRARY_NAME}_dummy PUBLIC ${DUMMY_OUTPUT_PATH} ${Qt5Qml_PRIVATE_INCLUDE_DIRS})

    file(GLOB_RECURSE IPC_GENERATED_FILES ${IPC_OUTPUT_PATH}/*.*)
    file(GLOB_RECURSE IPC_GENERATED_FILES_HEADERS ${IPC_OUTPUT_PATH}/*.h)
    qt5_wrap_cpp(IPC_GENERATED_FILES_HEADERS_MOCS ${IPC_GENERATED_FILES_HEADERS} TARGET ${LIBRARY_NAME}_ipc)
    facelift_add_aggregator_library(${LIBRARY_NAME}_ipc "${IPC_GENERATED_FILES};${IPC_GENERATED_FILES_HEADERS};${IPC_GENERATED_FILES_HEADERS_MOCS}")
    target_link_libraries(${LIBRARY_NAME}_ipc ${LIBRARY_NAME}_api IPCLib)
    target_include_directories(${LIBRARY_NAME}_ipc PUBLIC ${IPC_OUTPUT_PATH})

    add_library(${LIBRARY_NAME} INTERFACE)
    target_link_libraries(${LIBRARY_NAME} INTERFACE ${LIBRARY_NAME}_api ${LIBRARY_NAME}_dummy ${LIBRARY_NAME}_ipc)

    install(TARGETS ${LIBRARY_NAME}_api ${LIBRARY_NAME}_dummy ${LIBRARY_NAME}_ipc DESTINATION ${CMAKE_INSTALL_LIBDIR})

endfunction()


function(facelift_set_qml_implementation_path LIBRARY_NAME PATH)
    set_target_properties(${LIBRARY_NAME}_api PROPERTIES
        COMPILE_DEFINITIONS "QML_MODEL_LOCATION=${PATH}"
    )
endfunction()
