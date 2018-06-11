
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
    add_subdirectory(dbus)
    message("DBus IPC backend enabled")
    target_link_libraries(FaceliftIPCLib FaceliftIPCLibDBus)
endif()

