
function(add_example_script NAME MAIN_QML_FILE)
    set(QML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_QML_FILE})
    configure_file(${PROJECT_SOURCE_DIR}/examples/launch-example.sh ${PROJECT_BINARY_DIR}/examples/launch-${NAME}.sh @ONLY)
endfunction()


add_subdirectory(AddressBook)
add_subdirectory(Tuner)
add_subdirectory(Mediaplayer)
add_subdirectory(Basic)

if(FACELIFT_ENABLE_IPC)
add_subdirectory(IPC)
endif()