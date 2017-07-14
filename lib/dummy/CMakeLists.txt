set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt5_wrap_ui(UI_FILES dummymodelsmainwindow.ui)
qt5_wrap_ui(UI_FILES dummymodelpanel.ui)

add_library_simple(DummyModelLib)
target_link_libraries(DummyModelLib ModelLib PropertyLib CommonLib Qt5::Gui Qt5::Widgets)
target_sources(DummyModelLib PRIVATE ${UI_FILES})
target_include_directories(DummyModelLib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
