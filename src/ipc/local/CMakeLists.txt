 
facelift_add_library(FaceliftIPCLocalLib
    SOURCES
        LocalIPC.cpp
        LocalIPC.h
    HEADERS
        LocalIPCProxy.h
        LocalIPCRegistry.h
        LocalIPC-serialization.h
        LocalIPCServiceAdapter.h
    LINK_LIBRARIES FaceliftIPCCommonLib
    PUBLIC_HEADER_BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}
)
