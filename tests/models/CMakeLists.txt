facelift_add_interface(ModelInterface INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interface)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

facelift_add_qml_plugin(ModelsTestsPluginCpp
   URI tests.models
   OUTPUT_BASE_DIRECTORY imports/cpp
   SOURCES plugin/ModelsTestsPlugin.cpp
   HEADERS plugin/ModelsTestsPlugin.h
   LINK_LIBRARIES ModelInterface
)
