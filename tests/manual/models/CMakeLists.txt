facelift_add_library(ModelTestLib
    HEADERS cpp/TestInterfaceCppImplementation.h
    SOURCES cpp/TestInterfaceCppImplementation.cpp
    PUBLIC_HEADER_BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cpp
    LINK_LIBRARIES TestInterface
)
