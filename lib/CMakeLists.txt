
file(GLOB_RECURSE CPP_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
qt5_wrap_cpp(MOCS ${HEADER_FILES})
add_library(QmlCppAPILib ${CPP_SOURCE_FILES} ${MOCS})
target_link_libraries(QmlCppAPILib Qt5::Core Qt5::Gui Qt5::Qml Qt5::Quick Qt5::Widgets Qt5::DBus)

target_include_directories(QmlCppAPILib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(IPCLib INTERFACE)
target_include_directories(IPCLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/ipc)
