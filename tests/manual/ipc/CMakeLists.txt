if (FACELIFT_DBUS_IPC_ENABLED)

    facelift_add_executable(test_ipc_app
        SOURCES test_ipc.cpp
        LINK_LIBRARIES TestInterface Qt5::Core
    )
    facelift_add_test(test-ipc
        SOURCES TestIPC.cpp
        HEADERS TestIPC.h
        LINK_LIBRARIES TestInterface ModelTestLib Qt5::Core
    )

endif()
