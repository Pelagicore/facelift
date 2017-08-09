file(GLOB_RECURSE CPP_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

facelift_add_qml_plugin(TestPlugin facelift.test 1 0 "${CPP_SOURCE_FILES}" "${HEADER_FILES}")
target_link_libraries(TestPlugin TestGenerated)
#target_include_directories(TestPlugin INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/AddressBook)

add_example_script(test ui/TestUI.qml)
