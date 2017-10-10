
facelift_add_library(FaceliftIPCLib
    SOURCES ipc.cpp
    HEADERS ipc.h
    LINK_LIBRARIES FaceliftModelLib FaceliftPropertyLib FaceliftCommonLib Qt5::DBus
)
