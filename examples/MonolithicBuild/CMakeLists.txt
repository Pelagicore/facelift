cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

project(MyProject VERSION 1.0)

find_package(facelift)

facelift_add_library(MyLibrary1
    SOURCES
        MySource1.cpp
#        ....
    HEADERS
        MyHeader1.h
#        ....
    MONOLITHIC_SUPPORTED
)

facelift_add_library(MyLibrary2
    SOURCES
        MySource2.cpp
#        ....
    HEADERS
        MyHeader2.h
#        ....
    LINK_LIBRARIES
        MyLibrary1
    MONOLITHIC_SUPPORTED
)

facelift_export_project()

