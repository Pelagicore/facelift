cmake_minimum_required(VERSION 3.1)

project(FaceLiftBuildTest)

find_package(Multimake HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../multimake)

set(SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

add_cmake_external_project(facelift "${SOURCE_ROOT}" "" "")
add_cmake_external_project(facelift-package-export "${SOURCE_ROOT}/examples/external/export" "facelift" "")
add_cmake_external_project(facelift-package-import "${SOURCE_ROOT}/examples/external/import" "facelift-package-export" "")
