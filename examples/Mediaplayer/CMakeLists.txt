
facelift_add_interface(MediaPlayerInterface
    INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interface
)

#facelift_set_qml_implementation_path(MediaPlayerGenerated "${CMAKE_CURRENT_SOURCE_DIR}/models/qml")

facelift_add_qml_plugin(MediaPlayerPlugin
    URI mediaplayer
    SOURCES_GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    HEADERS_GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    LINK_LIBRARIES MediaPlayerInterface
)

add_example_script(mediaplayer ui/MediaPlayerApp.qml)
