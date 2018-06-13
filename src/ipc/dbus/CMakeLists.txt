
find_package(Qt5DBus QUIET)

if(Qt5DBus_FOUND)
    set(GENERATED_FILE_PATH ${CMAKE_CURRENT_BINARY_DIR}/Generated)
    facelift_generate_code(INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interfaces OUTPUT_PATH ${GENERATED_FILE_PATH})
    include_directories(${GENERATED_FILE_PATH}/ipc ${GENERATED_FILE_PATH}/types)
    facelift_add_library(FaceliftIPCLibDBus
        SOURCES ipc-dbus.cpp ipc-dbus-object-registry.cpp
        HEADERS ipc-dbus.h ipc-dbus-serialization.h ipc-dbus-object-registry.h
        SOURCES_GLOB_RECURSE ${GENERATED_FILE_PATH}/ipc/*.cpp ${GENERATED_FILE_PATH}/types/*.cpp
        HEADERS_GLOB_RECURSE ${GENERATED_FILE_PATH}/ipc/*.h ${GENERATED_FILE_PATH}/types/*.h
        LINK_LIBRARIES FaceliftIPCLibLocal Qt5::DBus
        PUBLIC_HEADER_BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..
    )
    target_compile_definitions(FaceliftIPCLibDBus PUBLIC DBUS_IPC_ENABLED)
endif()
