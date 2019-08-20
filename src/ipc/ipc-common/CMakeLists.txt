
facelift_add_library(FaceliftIPCCommonLib
    SOURCES
        ipc-common.cpp
        IPCProxyNewBase.cpp
        ModuleIPCBase.cpp
        ipc-serialization.cpp
    HEADERS
        ipc-common.h
        IPCProxyNewBase.h
        IPCServiceAdapterBase.h
        ModuleIPCBase.h
        ipc-serialization.h
    LINK_LIBRARIES FaceliftModelLib FaceliftCommonLib
)
