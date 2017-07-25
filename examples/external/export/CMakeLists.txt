
project(test)

include(GNUInstallDirs)    # for standard installation locations

cmake_minimum_required(VERSION 3.1)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(Qt5DBus REQUIRED)

find_package(facelift REQUIRED)

facelift_add_package(AddressBookGenerated addressbook ${CMAKE_CURRENT_SOURCE_DIR}/../../AddressBook/interface)

# Export package so that other packages can import our package using "facelift_export_package"
facelift_export_package(addressbook)
