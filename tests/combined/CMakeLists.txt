facelift_add_interface(CombinedInterface INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interface)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

facelift_add_qml_plugin(CombinedTestsPluginCpp
   URI tests.combined
   OUTPUT_BASE_DIRECTORY import/cpp
   SOURCES plugin/CombinedTestsPlugin.cpp
   LINK_LIBRARIES CombinedInterface
)

facelift_add_qml_plugin(CombinedTestsPluginQml
   URI tests.combined
   OUTPUT_BASE_DIRECTORY import/qml
   SOURCES plugin/CombinedTestsPlugin.cpp
   LINK_LIBRARIES CombinedInterface
)
target_compile_definitions(CombinedTestsPluginQml PRIVATE -DQML_IMPL_LOCATION=${CMAKE_CURRENT_SOURCE_DIR})
