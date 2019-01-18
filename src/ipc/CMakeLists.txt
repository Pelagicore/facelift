
facelift_add_library(FaceliftIPCLibLocal
    SOURCES ipc-common/ipc-common.cpp ipc-common/IPCProxyBase.cpp
    HEADERS ipc-common/ipc-common.h ipc-common/IPCProxyBase.h ipc-common/IPCServiceAdapterBase.h
    LINK_LIBRARIES FaceliftModelLib FaceliftPropertyLib FaceliftCommonLib FaceliftQMLModelLib
)

facelift_add_library(FaceliftIPCLib
    SOURCES facelift-ipc.cpp
    HEADERS facelift-ipc.h
    LINK_LIBRARIES FaceliftIPCLibLocal
)

if(FACELIFT_DBUS_IPC_ENABLED)
    add_subdirectory(dbus)
    message("DBus IPC backend enabled")
    target_link_libraries(FaceliftIPCLib FaceliftIPCLibDBus)
endif()

