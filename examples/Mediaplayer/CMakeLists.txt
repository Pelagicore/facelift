
facelift_add_package(MediaPlayerGenerated mediaplayer ${CMAKE_CURRENT_SOURCE_DIR}/interface)

facelift_set_qml_implementation_path(MediaPlayerGenerated "${CMAKE_CURRENT_SOURCE_DIR}/models/qml")

file(GLOB_RECURSE CPP_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

facelift_add_qml_plugin(MediaPlayerPlugin mediaplayer 1 0 "${CPP_SOURCE_FILES}" "${HEADER_FILES}")
target_link_libraries(MediaPlayerPlugin MediaPlayerGenerated)
target_include_directories(MediaPlayerPlugin INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Mediaplayer)

add_example_script(mediaplayer ui/MediaPlayerApp.qml)
