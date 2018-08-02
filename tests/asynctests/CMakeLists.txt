facelift_add_interface(AsyncTestInterface INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interface)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

facelift_add_qml_plugin(AsyncTestsPluginCpp
   URI tests.asynctests
   OUTPUT_BASE_DIRECTORY imports/cpp
   SOURCES plugin/AsyncTestsPlugin.cpp
   LINK_LIBRARIES AsyncTestInterface
)

facelift_add_qml_plugin(AsyncTestsPluginQml
   URI tests.asynctests
   OUTPUT_BASE_DIRECTORY imports/qml
   SOURCES plugin/AsyncTestsPlugin.cpp
   LINK_LIBRARIES AsyncTestInterface
)
target_compile_definitions(AsyncTestsPluginQml PRIVATE -DQML_IMPL_LOCATION=${CMAKE_CURRENT_SOURCE_DIR})
