
find_package(GTest)
if(${GTEST_FOUND})
    find_library(GMOCK_LIBRARY gmock REQUIRED HINTS "${CMAKE_SYSTEM_PREFIX_PATH}")
    if(NOT ${GMOCK_LIBRARY} STREQUAL "GMOCK_LIBRARY-NOTFOUND")
        add_library(GTest::GMock UNKNOWN IMPORTED)
        set_target_properties(GTest::GMock PROPERTIES IMPORTED_LOCATION ${GMOCK_LIBRARY})
    else()
        message(WARNING "Google test/mock not found.")
    endif()

    find_package(Qt5 COMPONENTS Test REQUIRED)
    if(${QT5TEST_NOTFOUND})
        message(ERROR "Required package Qt5Test not found.")
    endif()
    find_package(Threads REQUIRED)
    if(${THREADS_NOTFOUND})
        message(ERROR "Required package Threads not found.")
    endif()
    set(FACELIFT_GTEST_LIBRARIES ${GTEST_BOTH_LIBRARIES} GTest::GMock Qt5::Test Threads::Threads)
    include_directories(${GTEST_INCLUDE_DIRS})

    facelift_add_test(UnitTests
        SOURCES FaceliftUtilsTest.cpp
        LINK_LIBRARIES
            ${FACELIFT_GTEST_LIBRARIES}
            FaceliftCommonLib
    )

    facelift_add_test(UnitTestsObserver
        SOURCES FaceliftObserverTest.cpp
        LINK_LIBRARIES
            ${FACELIFT_GTEST_LIBRARIES}
            FaceliftIPCCommonLib
    )
else()
    message(WARNING "Required package google test not found!")
endif()

