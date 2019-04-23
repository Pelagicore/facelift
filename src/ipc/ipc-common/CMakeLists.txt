
facelift_add_library(FaceliftIPCCommonLib
    SOURCES
        ipc-common.cpp
        IPCProxyBase.cpp
        ModuleIPCBase.cpp
        ipc-serialization.cpp
    HEADERS
        ipc-common.h
        IPCProxyBase.h
        IPCServiceAdapterBase.h
        ModuleIPCBase.h
        ipc-serialization.h
    LINK_LIBRARIES FaceliftModelLib FaceliftCommonLib
)
