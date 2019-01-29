include_directories(${CMAKE_CURRENT_SOURCE_DIR})

facelift_add_interface(TestInterface
    INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interface
)

facelift_add_executable(test1
    HEADERS test1.h
    SOURCES test1.cpp
    LINK_LIBRARIES FaceliftModelLib
)

add_subdirectory(models)
add_subdirectory(ipc)
add_subdirectory(property)

facelift_add_qml_plugin(TestPlugin
   URI facelift.test
   SOURCES_GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/plugin/*.cpp
   HEADERS_GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/plugin/*.h
   LINK_LIBRARIES ModelTestLib
)

# Uncomment to use QML backend implementation:
# target_compile_definitions(TestPlugin PRIVATE -DQML_IMPL_LOCATION=${CMAKE_CURRENT_SOURCE_DIR})

set(QML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/ui/TestUI.qml)
configure_file(${PROJECT_SOURCE_DIR}/examples/launch-example.sh ${PROJECT_BINARY_DIR}/tests/launch-test.sh @ONLY)
