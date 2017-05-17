qt5_wrap_cpp(MOCS test1.h TARGET test1)
add_executable(test1 ${MOCS} test1.cpp)
target_link_libraries(test1 ModelLib PropertyLib IPCLib)
