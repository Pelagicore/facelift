# Build and install a QML plugin
function(add_qml_plugin PLUGIN_NAME URI PLUGIN_MAJOR_VERSION PLUGIN_MINOR_VERSION SOURCE_FILES HEADERS)

    string(REPLACE "." "/" PLUGIN_PATH ${URI})

    qt5_wrap_cpp(MOCS_PLUGIN ${HEADERS} OPTIONS -Muri=${URI} TARGET ${PLUGIN_NAME})

    add_library(${PLUGIN_NAME} SHARED ${SOURCE_FILES} ${MOCS_PLUGIN})
    target_link_libraries(${PLUGIN_NAME} Qt5::Core Qt5::Qml Qt5::Quick)

    target_include_directories(${PLUGIN_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

    set(INSTALL_PATH imports/${PLUGIN_PATH})

    set_target_properties(${PLUGIN_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${INSTALL_PATH}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${INSTALL_PATH}
    )

    install(TARGETS ${PLUGIN_NAME} DESTINATION ${INSTALL_PATH})

    install(FILES ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/qmldir DESTINATION ${INSTALL_PATH})
    file(WRITE ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/qmldir "module ${URI}\nplugin ${PLUGIN_NAME}\ntypeinfo plugins.qmltypes")

    if (ENABLE_QMLPLUGINDUMP)
        add_custom_command(
            OUTPUT  ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/plugins.qmltypes
            COMMAND ${_qt5Core_install_prefix}/bin/qmlplugindump -noinstantiate ${URI} ${PLUGIN_MAJOR_VERSION}.${PLUGIN_MINOR_VERSION} ${CMAKE_BINARY_DIR}/imports > ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/plugins.qmltypes
            DEPENDS ${PLUGIN_NAME}
        )
        add_custom_target("generate_qmltypes_${PLUGIN_NAME}" ALL DEPENDS ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/plugins.qmltypes)
        add_dependencies("generate_qmltypes_${PLUGIN_NAME}" ${PLUGIN_NAME})
        install(FILES ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/plugins.qmltypes DESTINATION ${INSTALL_PATH})
    endif()

endfunction()


