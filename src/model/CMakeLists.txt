
facelift_add_library(FaceliftModelLib
    SOURCES FaceliftModel.cpp QMLFrontend.cpp ServiceWrapper.cpp StructureBase.cpp ModuleBase.cpp QMLModel.cpp FaceliftProperty.cpp
    HEADERS FaceliftModel.h QMLFrontend.h ServiceWrapper.h StructureBase.h ModuleBase.h ServiceRegistry.h StructQObjectWrapper.h QMLModel.h FaceliftProperty.h FaceliftConversion.h FaceliftCommon.h FaceliftEnum.h
    LINK_LIBRARIES FaceliftCommonLib Qt5::Qml Qt5::Quick
)
