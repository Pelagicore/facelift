
facelift_add_library(FaceliftModelLib
    SOURCES
        FaceliftModel.cpp
        QMLAdapter.cpp
        ServiceWrapper.cpp
        StructureBase.cpp
        ModuleBase.cpp
        QMLModel.cpp
        FaceliftProperty.cpp
        FaceliftImplementationBase.cpp
        FaceliftEnum.cpp
    HEADERS
        FaceliftModel.h
        QMLAdapter.h
        ServiceWrapper.h
        StructureBase.h
        ModuleBase.h
        ServiceRegistry.h
        StructQObjectWrapper.h
        QMLModel.h
        FaceliftProperty.h
        FaceliftConversion.h
        FaceliftCommon.h
        FaceliftEnum.h
        FaceliftImplementationBase.h
    LINK_LIBRARIES FaceliftCommonLib Qt5::Qml Qt5::Quick
)
