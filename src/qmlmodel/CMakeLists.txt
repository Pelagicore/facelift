
facelift_add_library(FaceliftQMLModelLib
    SOURCES QMLModel.cpp
    HEADERS QMLModel.h
    LINK_LIBRARIES FaceliftModelLib FaceliftPropertyLib Qt5::Qml Qt5::Quick
)
