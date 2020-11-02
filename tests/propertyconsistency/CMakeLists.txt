if (TARGET FaceliftIPCLibDBus)

    facelift_add_interface(property_consistency_gen
        INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interface)

    facelift_add_executable(property-consistency-test-client
        HEADERS
            client.h
        SOURCES
            client.cpp
        LINK_LIBRARIES
            Qt5::Core
            property_consistency_gen
    )

    facelift_add_test(property-consistency-test
        HEADERS
            server.h
        SOURCES
            server.cpp
        PRIVATE_DEFINITIONS CLIENT_EXECUTABLE_LOCATION="$<TARGET_FILE:property-consistency-test-client>"
        LINK_LIBRARIES
            Qt5::Core
            property_consistency_gen
    )

endif()
