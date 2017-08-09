
facelift_add_package(AddressBookGenerated "facelift.example.addressbook" ${CMAKE_CURRENT_SOURCE_DIR}/interface)

add_definitions(-DQML_MODEL_LOCATION=${CMAKE_CURRENT_SOURCE_DIR})

#facelift_set_qml_implementation_path(AddressBookGenerated "${CMAKE_CURRENT_SOURCE_DIR}/models/qml")

file(GLOB_RECURSE CPP_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

facelift_add_qml_plugin(AddressBookModelPlugin facelift.example.addressbook 1 0 "${CPP_SOURCE_FILES}" "${HEADER_FILES}")
target_link_libraries(AddressBookModelPlugin AddressBookGenerated)
target_include_directories(AddressBookModelPlugin INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/AddressBook)

add_example_script(addressbook ui/AddressBookApp.qml)
add_example_script(addressbook-dummy ui/AddressBookDummyApp.qml)
