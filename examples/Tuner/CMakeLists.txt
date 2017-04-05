
add_qface_package(TunerGenerated ${CMAKE_CURRENT_SOURCE_DIR}/interface)

file(GLOB_RECURSE CPP_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_qml_plugin(TunerModelPlugin tuner 1 0 "${CPP_SOURCE_FILES}" "${HEADER_FILES}" "TunerGenerated")
target_include_directories(TunerModelPlugin INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Tuner)

set_target_properties(TunerModelPlugin PROPERTIES
    COMPILE_DEFINITIONS "TUNERVIEWMODEL_QML_LOCATION=${CMAKE_CURRENT_SOURCE_DIR}/models/TunerViewModelImpl.qml"
)
