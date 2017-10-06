
facelift_add_library(DummyModelLib 
    SOURCES ServiceMonitor.cpp DummyModel.cpp
    HEADERS ServiceMonitor.h DummyModel.h ControlWidgets.h
    UI_FILES dummymodelsmainwindow.ui dummymodelpanel.ui servicemonitorpanel.ui servicemonitormanagerwindow.ui
    LINK_LIBRARIES ModelLib PropertyLib CommonLib Qt5::Gui Qt5::Widgets
)

target_include_directories(DummyModelLib PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${Qt5Qml_PRIVATE_INCLUDE_DIRS})
