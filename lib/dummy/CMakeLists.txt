
facelift_add_library(FaceliftDummyModelLib
    SOURCES ServiceMonitor.cpp DummyModel.cpp
    HEADERS ServiceMonitor.h DummyModel.h ControlWidgets.h
    UI_FILES dummymodelsmainwindow.ui dummymodelpanel.ui servicemonitorpanel.ui servicemonitormanagerwindow.ui
    LINK_LIBRARIES FaceliftModelLib FaceliftPropertyLib FaceliftCommonLib Qt5::Gui Qt5::Widgets
)

target_include_directories(FaceliftDummyModelLib PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${Qt5Qml_PRIVATE_INCLUDE_DIRS})
