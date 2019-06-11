#project(facelift_codegen VERSION 1.0.0)

#cmake_minimum_required(VERSION 3.1)

include(GNUInstallDirs)    # for standard installation locations

add_executable(facelift-codegen IMPORTED GLOBAL)
set_target_properties(facelift-codegen PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/facelift/facelift-codegen.py)

set(INSTALLATION_PATH ${CMAKE_INSTALL_BINDIR}/host-tools/facelift)

install(DIRECTORY facelift/ DESTINATION ${INSTALLATION_PATH} USE_SOURCE_PERMISSIONS)

get_target_property(FACELIFT_CODEGEN_LOCATION facelift-codegen LOCATION)

file(RELATIVE_PATH FACELIFT_CODEGEN_RELATIVE_LOCATION /${CMAKE_BINARY_DIR} /${FACELIFT_CODEGEN_LOCATION})
configure_file(faceliftConfig-codegen.cmake.in ${CMAKE_BINARY_DIR}/faceliftConfig-codegen.cmake @ONLY)

file(RELATIVE_PATH FACELIFT_CODEGEN_RELATIVE_LOCATION /lib/cmake/${PROJECT_NAME} /${INSTALLATION_PATH}/facelift-codegen.py)
configure_file(faceliftConfig-codegen.cmake.in ${CMAKE_BINARY_DIR}/faceliftConfig-codegen.installed.cmake @ONLY)
