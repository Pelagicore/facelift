
add_subdirectory(ipc-common)

facelift_add_library(FaceliftIPCLib
    SOURCES
        facelift-ipc.cpp
    HEADERS
        facelift-ipc.h
    LINK_LIBRARIES FaceliftIPCCommonLib
)

if(FACELIFT_DBUS_IPC_ENABLED)
    add_subdirectory(dbus)
    message("DBus IPC backend enabled")
    target_link_libraries(FaceliftIPCLib FaceliftIPCLibDBus)
endif()

