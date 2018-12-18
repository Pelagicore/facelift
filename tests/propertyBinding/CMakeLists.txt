facelift_add_interface(PropertyBindingInterface INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interface)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

facelift_add_qml_plugin(PropertyBindingCpp
    URI tests.propertybinding
    OUTPUT_BASE_DIRECTORY imports/cpp
    SOURCES plugin/PropertyBindingPlugin.cpp
            impl/cpp/PropertyBindingInterfaceCppImplementation.h
    LINK_LIBRARIES PropertyBindingInterface)

facelift_add_qml_plugin(PropertyBindingQml
    URI tests.propertybinding
    OUTPUT_BASE_DIRECTORY imports/qml
    SOURCES plugin/PropertyBindingPlugin.cpp
            impl/cpp/PropertyBindingInterfaceCppImplementation.h
    LINK_LIBRARIES PropertyBindingInterface)

target_compile_definitions(PropertyBindingQml PRIVATE -DQML_IMPL_LOCATION=${CMAKE_CURRENT_SOURCE_DIR})

