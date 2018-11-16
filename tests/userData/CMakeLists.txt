facelift_add_interface(UserDataInterface INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interface)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

facelift_add_qml_plugin(UserDataTestsPluginCpp
   URI tests.userData
   OUTPUT_BASE_DIRECTORY imports/cpp
   SOURCES plugin/UserDataTestsPlugin.cpp
   HEADERS impl/cpp/UserDataTestsCppImplementation.h
   LINK_LIBRARIES UserDataInterface
)
