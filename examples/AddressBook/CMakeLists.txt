
facelift_add_package(AddressBookGenerated "facelift.example.addressbook" ${CMAKE_CURRENT_SOURCE_DIR}/interface)

add_definitions(-DQML_MODEL_LOCATION=${CMAKE_CURRENT_SOURCE_DIR})

#facelift_set_qml_implementation_path(AddressBookGenerated "${CMAKE_CURRENT_SOURCE_DIR}/models/qml")

facelift_add_qml_plugin(AddressBookModelPlugin
    URI facelift.example.addressbook
    SOURCES_GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    HEADERS_GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    LINK_LIBRARIES AddressBookGenerated
)

add_example_script(addressbook ui/AddressBookApp.qml)
add_example_script(addressbook-dummy ui/AddressBookDummyApp.qml)
