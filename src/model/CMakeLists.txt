
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
        FaceliftStringConversion.cpp
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
        FaceliftStringConversion.h
        FaceliftConversion.h
        FaceliftCommon.h
        FaceliftEnum.h
        FaceliftImplementationBase.h
        FaceliftQMLUtils.h
    LINK_LIBRARIES Qt5::Qml Qt5::Quick
    MONOLITHIC_LINK_LIBRARIES FaceliftCommonLib
    MONOLITHIC_SUPPORTED
)
