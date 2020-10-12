 
facelift_add_library(FaceliftIPCLocalLib
    SOURCES
        LocalIPCMessage.cpp
        LocalIPCProxyBase.cpp
        LocalIPCProxyBinder.cpp
        LocalIPCRegistry.cpp
        LocalIPCServiceAdapterBase.cpp
    HEADERS
        LocalIPCProxyBinder.h
        LocalIPCServiceAdapterBase.h
        LocalIPCRegistry.h
    HEADERS_NO_MOC
        FaceliftIPCCommon.h
        LocalIPCMessage.h
        LocalIPCProxy.h
        LocalIPC-serialization.h
        LocalIPCProxyBase.h
        LocalIPCRequestHandler.h
        LocalIPCServiceAdapter.h
    LINK_LIBRARIES FaceliftIPCCommonLib FaceliftModelLib
    PUBLIC_HEADER_BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}
    MONOLITHIC_SUPPORTED
)
