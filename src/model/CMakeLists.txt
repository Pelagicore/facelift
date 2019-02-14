
facelift_add_library(FaceliftModelLib
    SOURCES 
        FaceliftModel.cpp
        FaceliftProperty.cpp
        ModuleBase.cpp
        QMLFrontend.cpp
        QMLModel.cpp
        ServiceWrapper.cpp
        StructureBase.cpp
    HEADERS 
        FaceliftCommon.h
        FaceliftConversion.h
        FaceliftEnum.h
        FaceliftModel.h
        FaceliftProperty.h
        ModuleBase.h
        QMLFrontend.h
        QMLModel.h
        ServiceRegistry.h
        ServiceWrapper.h
        StructQObjectWrapper.h
        StructureBase.h
    LINK_LIBRARIES FaceliftCommonLib Qt5::Qml Qt5::Quick
)
