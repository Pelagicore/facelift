 
facelift_add_library(FaceliftIPCLocalLib
    SOURCES
        LocalIPCMessage.cpp
        LocalIPCProxyBase.cpp
        LocalIPCProxyBinder.cpp
        LocalIPCRegistry.cpp
        LocalIPCServiceAdapterBase.cpp
    HEADERS
        LocalIPCMessage.h
        LocalIPCProxyBase.h
        LocalIPCProxyBinder.h
        LocalIPCRequestHandler.h
        LocalIPCServiceAdapterBase.h
        LocalIPCProxy.h
        LocalIPCRegistry.h
        LocalIPC-serialization.h
        LocalIPCServiceAdapter.h
        FaceliftIPCCommon.h
    LINK_LIBRARIES FaceliftIPCCommonLib FaceliftModelLib
    PUBLIC_HEADER_BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}
    MONOLITHIC_SUPPORTED
)
