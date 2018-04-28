
facelift_add_interface(TunerInterface
    INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interface
)

#facelift_set_qml_implementation_path(TunerGenerated "${CMAKE_CURRENT_SOURCE_DIR}/models/qml")

facelift_add_qml_plugin(TunerModelPlugin
    URI tuner
    SOURCES_GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    HEADERS_GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    LINK_LIBRARIES TunerInterface
)

add_example_script(tuner ui/TunerApp.qml)
add_example_script(tunerdummy ui/TunerAppWithDummy.qml)
