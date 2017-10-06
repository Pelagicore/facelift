
facelift_add_library(IPCLib 
    SOURCES ipc.cpp
    HEADERS ipc.h
    LINK_LIBRARIES ModelLib PropertyLib CommonLib Qt5::DBus
)
