
facelift_add_interface(BenchmarkingInterface
    INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interface
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

facelift_add_qml_plugin(BenchmarkingPlugin
    URI tests.benchmarking
    OUTPUT_BASE_DIRECTORY imports/cpp
    SOURCES BenchmarkingPlugin.cpp
    LINK_LIBRARIES BenchmarkingInterface
)

