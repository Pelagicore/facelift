facelift_add_package(TestGenerated "facelift.test" ${CMAKE_CURRENT_SOURCE_DIR}/interface)

qt5_wrap_cpp(MOCS test1.h TARGET test1)
add_executable(test1 ${MOCS} test1.cpp)
target_link_libraries(test1 FaceliftModelLib FaceliftPropertyLib)

if(FACELIFT_ENABLE_IPC)
    add_subdirectory(ipc)
endif()

