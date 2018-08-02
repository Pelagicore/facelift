facelift_add_interface(AsyncFunctionsInterface INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interface)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

facelift_add_qml_plugin(AsyncFunctionsPluginCpp
   URI tests.asyncfunctions
   OUTPUT_BASE_DIRECTORY imports/cpp
   SOURCES plugin/AsyncFunctionsPlugin.cpp
   LINK_LIBRARIES AsyncFunctionsInterface
)

