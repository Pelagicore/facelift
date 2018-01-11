add_definitions(-DENABLE_DESKTOP_TOOLS)

facelift_add_qml_plugin(FaceliftPlugin
    URI facelift
    SOURCES FaceliftPlugin.cpp
    HEADERS FaceliftPlugin.h
    LINK_LIBRARIES FaceliftDesktopDevTools
)
