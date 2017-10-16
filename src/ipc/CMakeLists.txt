
if(FACELIFT_ENABLE_DBUS_IPC OR (NOT FACELIFT_DISABLE_DBUS_IPC))
    find_package(Qt5DBus QUIET)

    if(Qt5DBus_FOUND)
        facelift_add_library(FaceliftIPCLib
            SOURCES ipc.cpp
            HEADERS ipc.h
            LINK_LIBRARIES FaceliftModelLib FaceliftPropertyLib FaceliftCommonLib Qt5::DBus
        )
    endif()

endif()
