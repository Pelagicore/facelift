
facelift_add_library(FaceliftModelLib
    SOURCES Model.cpp
    HEADERS Model.h QMLFrontend.h
    LINK_LIBRARIES FaceliftCommonLib Qt5::Qml Qt5::Quick
)
