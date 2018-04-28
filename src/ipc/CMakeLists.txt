facelift_add_library(FaceliftIPCLibLocal
    SOURCES ipc-common/ipc-common.cpp local/ipc-local.cpp
    HEADERS ipc-common/ipc-common.h local/ipc-local.h
    LINK_LIBRARIES FaceliftModelLib FaceliftPropertyLib FaceliftCommonLib FaceliftQMLModelLib
)

facelift_add_library(FaceliftIPCLib
    SOURCES ipc.cpp
    HEADERS ipc.h
    LINK_LIBRARIES FaceliftIPCLibLocal
)

if(FACELIFT_ENABLE_DBUS_IPC OR (NOT FACELIFT_DISABLE_DBUS_IPC))
    find_package(Qt5DBus QUIET)

    if(Qt5DBus_FOUND)
        facelift_add_library(FaceliftIPCLibDBus
            SOURCES dbus/ipc-dbus.cpp
            HEADERS dbus/ipc-dbus.h dbus/ipc-dbus-serialization.h
            LINK_LIBRARIES FaceliftIPCLibLocal Qt5::DBus
        )

        message("DBus IPC backend enabled")
        target_compile_definitions(FaceliftIPCLib PUBLIC DBUS_IPC_ENABLED)
        target_link_libraries(FaceliftIPCLib FaceliftIPCLibDBus)

    endif()

endif()

