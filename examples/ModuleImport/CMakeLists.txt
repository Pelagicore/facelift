facelift_add_interface(MainInterface
                       INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interface
                       IMPORT_FOLDERS ${CMAKE_CURRENT_SOURCE_DIR}/interface)

facelift_add_qml_plugin(ModuleImportPlugin
                        URI mainmodule
                        SOURCES ModuleImportPlugin.cpp
                        HEADERS ModuleImportPlugin.h model/cpp/MainInterfaceCppImplementation.h
                        LINK_LIBRARIES MainInterface)

add_example_script(module-import ui/MyApp.qml)
