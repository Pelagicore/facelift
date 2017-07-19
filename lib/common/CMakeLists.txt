set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library_simple(CommonLib)
target_link_libraries(CommonLib Qt5::Gui Qt5::Widgets)
target_include_directories(CommonLib PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
