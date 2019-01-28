
facelift_add_library(FaceliftModelLib
    SOURCES FaceliftModel.cpp QMLFrontend.cpp ServiceWrapper.cpp StructureBase.cpp ModuleBase.cpp
    HEADERS FaceliftModel.h QMLFrontend.h ServiceWrapper.h StructureBase.h ModuleBase.h ServiceRegistry.h
    LINK_LIBRARIES FaceliftCommonLib Qt5::Qml Qt5::Quick
)
