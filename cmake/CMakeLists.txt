#project(facelift_codegen VERSION 1.0.0)

#cmake_minimum_required(VERSION 3.1)

include(GNUInstallDirs)    # for standard installation locations

add_executable(unity_generator IMPORTED GLOBAL)
set_target_properties(unity_generator PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/facelift/unity_generator.py)

set(INSTALLATION_PATH ${CMAKE_INSTALL_BINDIR}/host-tools/facelift)

install(DIRECTORY facelift/ DESTINATION ${INSTALLATION_PATH} USE_SOURCE_PERMISSIONS)

get_target_property(UNITY_GEN_LOCATION unity_generator LOCATION)

file(RELATIVE_PATH UNITY_GEN_RELATIVE_LOCATION /${CMAKE_BINARY_DIR} /${UNITY_GEN_LOCATION})
configure_file(faceliftUnityConfig.cmake.in ${CMAKE_BINARY_DIR}/faceliftUnityConfig.cmake @ONLY)

file(RELATIVE_PATH UNITY_GEN_RELATIVE_LOCATION /lib/cmake/${PROJECT_NAME} /${INSTALLATION_PATH}/unity_generator.py)
configure_file(faceliftUnityConfig.cmake.in ${CMAKE_BINARY_DIR}/faceliftUnityConfig.installed.cmake @ONLY)
