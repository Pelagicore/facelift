
facelift_add_interface(MyInterface
    INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interface IMPORT_FOLDERS ${CMAKE_CURRENT_SOURCE_DIR}/interface)

add_definitions(-DQML_MODEL_LOCATION=${CMAKE_CURRENT_SOURCE_DIR})

facelift_add_qml_plugin(MyPlugin   URI facelift.example.myinterface
    SOURCES_GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    HEADERS_GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    LINK_LIBRARIES MyInterface)

add_example_script(MyInterface ui/MyApp.qml)
