
facelift_add_library(QMLModelLib 
    SOURCES QMLModel.cpp
    HEADERS QMLModel.h
    LINK_LIBRARIES ModelLib PropertyLib Qt5::Qml Qt5::Quick
)
