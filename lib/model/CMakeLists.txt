
facelift_add_library(ModelLib 
    SOURCES Model.cpp
    HEADERS Model.h QMLFrontend.h
    LINK_LIBRARIES CommonLib Qt5::Qml Qt5::Quick
)
