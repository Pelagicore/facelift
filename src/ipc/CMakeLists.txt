
facelift_add_library(FaceliftIPCLibLocal
    SOURCES ipc-common/ipc-common.cpp ipc-common/IPCProxyBase.cpp ipc-common/ModuleIPCBase.cpp
    HEADERS ipc-common/ipc-common.h ipc-common/IPCProxyBase.h ipc-common/IPCServiceAdapterBase.h ipc-common/ModuleIPCBase.h
    LINK_LIBRARIES FaceliftModelLib FaceliftCommonLib
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

