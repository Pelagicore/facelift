
find_package(Qt5 REQUIRED Widgets)

set_property(GLOBAL PROPERTY FACELIFT_DEV_TOOLS_ENABLED ON)

facelift_add_library(FaceliftDesktopDevTools
    SOURCES ServiceMonitor.cpp DummyModel.cpp
    HEADERS ServiceMonitor.h DummyModel.h ControlWidgets.h ServiceMonitorQMLComponent.h
    UI_FILES dummymodelsmainwindow.ui dummymodelpanel.ui servicemonitorpanel.ui servicemonitormanagerwindow.ui
    LINK_LIBRARIES FaceliftModelLib FaceliftCommonLib Qt5::Gui Qt5::Widgets
)

target_include_directories(FaceliftDesktopDevTools PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${Qt5Qml_PRIVATE_INCLUDE_DIRS})
