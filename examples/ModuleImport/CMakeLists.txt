facelift_add_interface(ModuleImport
    INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interface
    IMPORT_FOLDERS  ${CMAKE_CURRENT_SOURCE_DIR}/interface)

facelift_add_qml_plugin(ModuleImportPlugin  URI moduleimport
   SOURCES ModuleImportPlugin.cpp
   HEADERS ModuleImportPlugin.h  model/cpp/MainModuleCppImplementation.h
   LINK_LIBRARIES ModuleImport)

add_example_script(ModuleImport ui/MyApp.qml)
