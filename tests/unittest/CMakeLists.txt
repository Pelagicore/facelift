
find_package(GTest)
if(${GTEST_FOUND})
    find_library(GMOCK_LIBRARY gmock REQUIRED HINTS "${CMAKE_SYSTEM_PREFIX_PATH}")
    if(NOT ${GMOCK_LIBRARY} STREQUAL "GMOCK_LIBRARY-NOTFOUND")
        add_library(GTest::GMock UNKNOWN IMPORTED)
        set_target_properties(GTest::GMock PROPERTIES IMPORTED_LOCATION ${GMOCK_LIBRARY})
    else()
        message(WARNING "Google test/mock not found.")
    endif()

    find_package(Threads REQUIRED)
    set(FACELIFT_GTEST_LIBRARIES ${GTEST_BOTH_LIBRARIES} GTest::GMock Threads::Threads)
    include_directories(${GTEST_INCLUDE_DIRS})

    facelift_add_test(UnitTests
        SOURCES FaceliftUtilsTest.cpp
        LINK_LIBRARIES ${FACELIFT_GTEST_LIBRARIES} FaceliftCommonLib)

else()
    message(WARNING "Required package google test not found!")
endif()

