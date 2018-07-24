
facelift_add_interface(AdvancedInterface
    INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interface
)

facelift_add_qml_plugin(AdvancedPlugin
    URI advanced
    SOURCES_GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    HEADERS_GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    LINK_LIBRARIES AdvancedInterface
)

add_example_script(advanced ui/AdvancedApp.qml)
add_example_script(advanced-server ui/AdvancedAppServer.qml)
add_example_script(advanced-server-in-process ui/AdvancedAppServerWithClient.qml)
