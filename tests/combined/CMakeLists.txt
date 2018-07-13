facelift_add_interface(CombinedInterface INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interface)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

facelift_add_qml_plugin(CombinedTestsPluginCpp
   URI tests.combined
   IMPORT_FOLDER import/cpp
   SOURCES plugin/cpp/CombinedTestsPlugin.cpp
   HEADERS plugin/cpp/CombinedTestsPlugin.h
   LINK_LIBRARIES CombinedInterface
)

add_definitions(-DQML_IMPL_LOCATION=${CMAKE_CURRENT_SOURCE_DIR})

facelift_add_qml_plugin(CombinedTestsPluginQml
   URI tests.combined
   IMPORT_FOLDER import/qml
   SOURCES plugin/qml/CombinedTestsPlugin.cpp
   HEADERS plugin/qml/CombinedTestsPlugin.h
   LINK_LIBRARIES CombinedInterface
)
