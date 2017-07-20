project(facelift)

cmake_minimum_required(VERSION 3.1)

include(GNUInstallDirs)    # for standard installation locations

install(DIRECTORY facelift DESTINATION ${CMAKE_INSTALL_BINDIR} USE_SOURCE_PERMISSIONS)

install(FILES facelift-codegen.py DESTINATION ${CMAKE_INSTALL_BINDIR} 
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
)

file(RELATIVE_PATH CODEGEN_RELATIVE_PATH /${CMAKE_CONFIG_INSTALLATION_PATH} /${CMAKE_INSTALL_BINDIR})

configure_file(faceliftConfig-codegen.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/faceliftConfig-codegen.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/faceliftConfig-codegen.cmake DESTINATION ${CMAKE_CONFIG_INSTALLATION_PATH})
