facelift_add_package(TestGenerated ${CMAKE_CURRENT_SOURCE_DIR}/interface)

qt5_wrap_cpp(MOCS test1.h TARGET test1)
add_executable(test1 ${MOCS} test1.cpp)
target_link_libraries(test1 ModelLib PropertyLib IPCLib)

add_subdirectory(ipc)
