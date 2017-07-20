
project(test)

cmake_minimum_required(VERSION 3.1)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(Qt5DBus REQUIRED)

find_package(facelift REQUIRED)

add_qface_package(AddressBookGenerated ${CMAKE_CURRENT_SOURCE_DIR}/../AddressBook/interface)
