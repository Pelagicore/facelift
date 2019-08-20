 
facelift_add_library(FaceliftIPCLocalLib
    SOURCES
        LocalIPC.cpp
    HEADERS
        LocalIPC.h
        LocalIPCProxy.h
        LocalIPCRegistry.h
        LocalIPC-serialization.h
        LocalIPCServiceAdapter.h
        IPCProxyBase.h
    LINK_LIBRARIES FaceliftIPCCommonLib
    PUBLIC_HEADER_BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}
)
