
facelift_add_package(TunerGenerated tuner ${CMAKE_CURRENT_SOURCE_DIR}/interface)

facelift_set_qml_implementation_path(TunerGenerated "${CMAKE_CURRENT_SOURCE_DIR}/models/qml")

file(GLOB_RECURSE CPP_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

facelift_add_qml_plugin(TunerModelPlugin tuner 1 0 "${CPP_SOURCE_FILES}" "${HEADER_FILES}")
target_link_libraries(TunerModelPlugin TunerGenerated)
target_include_directories(TunerModelPlugin INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Tuner)

add_example_script(tuner ui/TunerApp.qml)
add_example_script(tunerdummy ui/TunerAppWithDummy.qml)
