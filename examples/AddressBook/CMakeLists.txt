
facelift_add_interface(AddressBookInterface
    INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interface
)

add_definitions(-DQML_MODEL_LOCATION=${CMAKE_CURRENT_SOURCE_DIR})

facelift_add_qml_plugin(AddressBookModelPlugin
    URI facelift.example.addressbook
    SOURCES_GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    HEADERS_GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    LINK_LIBRARIES AddressBookInterface
)

add_example_script(addressbook ui/AddressBookApp.qml)
add_example_script(addressbook-monitor ui/AddressBookMonitor.qml)
add_example_script(addressbook-dummy ui/AddressBookDummyApp.qml)
