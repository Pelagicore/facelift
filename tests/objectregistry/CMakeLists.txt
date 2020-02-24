facelift_add_interface(tests_ipc
    INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interface)

facelift_add_executable(object-registry-test-client
    HEADERS
        client.h
    SOURCES
        client.cpp
    LINK_LIBRARIES
        Qt5::Core
        tests_ipc
        FaceliftIPCLibDBus
)

facelift_add_test(object-registry-communication
    HEADERS
        server.h
    SOURCES
        server.cpp
    PRIVATE_DEFINITIONS CLIENT_EXECUTABLE_LOCATION="$<TARGET_FILE:object-registry-test-client>"
    LINK_LIBRARIES
        Qt5::Core
        tests_ipc
        FaceliftIPCLibDBus
)



