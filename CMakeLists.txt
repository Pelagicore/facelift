project(QmlCppAPI)

cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

include("cmake/QMLCppAPIConfig.cmake")

# No undefined symbol allowed in our shared libraries
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


# Build a QML plugin
function(add_qml_plugin PLUGIN_NAME URI PLUGIN_MAJOR_VERSION PLUGIN_MINOR_VERSION SOURCE_FILES HEADERS)

    # All plugins are linked against Qt libraries (QML and Quick)
    string(REPLACE "." "/" PLUGIN_PATH ${URI})

    set(PLUGINREGISTER "plugin ${PLUGIN_NAME}")
    qt5_wrap_cpp(MOCS_PLUGIN ${HEADERS} OPTIONS -Muri=${URI})

    add_library(${PLUGIN_NAME} SHARED ${SOURCE_FILES} ${MOCS_PLUGIN})
    target_link_libraries(${PLUGIN_NAME} Qt5::Core Qt5::Gui Qt5::Qml Qt5::Quick)

    target_include_directories(${PLUGIN_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

    set(INSTALL_PATH imports/${PLUGIN_PATH})

    set_target_properties(${PLUGIN_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${INSTALL_PATH}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${INSTALL_PATH}
    )

    install(TARGETS ${PLUGIN_NAME} DESTINATION ${INSTALL_PATH})

    #Build the exports defines from the library path
    string(TOUPPER "${PLUGIN_PATH}_PLUGIN" EXPORT_DEFINE)
    string(REPLACE "/" "_" EXPORT_DEFINE ${EXPORT_DEFINE})

    set_target_properties(${PLUGIN_NAME} PROPERTIES
        COMPILE_DEFINITIONS  "PLUGIN_MINOR_VERSION=${PLUGIN_MINOR_VERSION};PLUGIN_MAJOR_VERSION=${PLUGIN_MAJOR_VERSION};${EXPORT_DEFINE}"
    )

    #write qmldir file
    set(QMLDIR_PLUGIN ${PLUGINREGISTER})
    set(QMLDIR_URI ${URI})

    install(FILES ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/qmldir DESTINATION ${INSTALL_PATH})
    set(REG_IMPORT "Q_IMPORT_PLUGIN(${PLUGIN_NAME})\n")
    set_property(GLOBAL APPEND_STRING PROPERTY STATIC_REGISTRATION_IMPORTS ${REG_IMPORT})

    file(WRITE ${CMAKE_BINARY_DIR}/${INSTALL_PATH}/qmldir "module ${QMLDIR_URI}\n${QMLDIR_PLUGIN}\ntypeinfo plugins.qmltypes")

endfunction()



#add_subdirectory(lib)
add_subdirectory(examples)
add_subdirectory(test)


file(GLOB_RECURSE FILES_TO_SHOW_IN_QTCREATOR *.qml *.js *.cmake qmldir *.yaml *.qdl)
add_custom_target(QFACE_SHOW_IN_QTCREATOR SOURCES ${FILES_TO_SHOW_IN_QTCREATOR})
