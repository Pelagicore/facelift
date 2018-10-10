facelift_add_interface(ReadyFlagInterface INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interface)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

facelift_add_qml_plugin(ReadyFlagPluginCpp
   URI tests.readyflag
   OUTPUT_BASE_DIRECTORY imports/cpp
   SOURCES plugin/ReadyFlagPlugin.cpp
   LINK_LIBRARIES ReadyFlagInterface
)

