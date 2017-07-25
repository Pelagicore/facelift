
project(test)

include(GNUInstallDirs)    # for standard installation locations

cmake_minimum_required(VERSION 3.1)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(Qt5DBus REQUIRED)

find_package(facelift REQUIRED)

facelift_import_package(AddressBookFaceliftPackage addressbook)

add_executable(test1 test.cpp)

target_link_libraries(test1 ${AddressBookFaceliftPackage})

install(TARGETS test1 DESTINATION ${CMAKE_INSTALL_BINDIR})
