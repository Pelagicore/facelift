facelift_add_qml_plugin(FaceliftPlugin
    URI facelift
    SOURCES FaceliftPlugin.cpp
    HEADERS FaceliftPlugin.h
)

if(FACELIFT_ENABLE_DESKTOP_DEV_TOOLS)
    target_link_libraries(FaceliftPlugin FaceliftDesktopDevTools)
    target_compile_definitions(FaceliftPlugin PRIVATE -DENABLE_DESKTOP_TOOLS)
endif()
