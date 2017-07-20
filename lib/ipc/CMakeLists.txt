set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library_simple(IPCLib)
target_link_libraries(IPCLib ModelLib PropertyLib CommonLib Qt5::DBus)

configure_file(ipc-config.h.in ${CMAKE_CURRENT_BINARY_DIR}/ipc-config.h @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ipc-config.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

target_include_directories(IPCLib
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
           $<INSTALL_INTERFACE:${HEADERS_INSTALLATION_LOCATION}>
)
