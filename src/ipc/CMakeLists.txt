
if(FACELIFT_ENABLE_DBUS_IPC OR (NOT FACELIFT_DISABLE_DBUS_IPC))
    find_package(Qt5DBus QUIET)

    if(Qt5DBus_FOUND)
        facelift_add_library(FaceliftIPCLib
            SOURCES ipc.cpp ipc-common/ipc-common.cpp local/ipc-local.cpp dbus/ipc-dbus.cpp
            HEADERS ipc.h local/ipc-local.h dbus/ipc-dbus.h dbus/ipc-dbus-serialization.h ipc-common/ipc-common.h
            LINK_LIBRARIES FaceliftModelLib FaceliftPropertyLib FaceliftCommonLib Qt5::DBus FaceliftQMLModelLib
        )
    endif()

endif()
