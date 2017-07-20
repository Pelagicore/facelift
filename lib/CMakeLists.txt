
set(HEADERS_INSTALLATION_LOCATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

function(add_library_simple LIBRARY_NAME)

    file(GLOB_RECURSE CPP_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
    file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
    qt5_wrap_cpp(MOCS ${HEADER_FILES} TARGET ${LIBRARY_NAME})
    add_library(${LIBRARY_NAME} SHARED ${CPP_SOURCE_FILES} ${MOCS})
    target_link_libraries(${LIBRARY_NAME} Qt5::Core Qt5::Qml Qt5::Quick Qt5::Quick)
    target_include_directories(${LIBRARY_NAME} 
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
               $<INSTALL_INTERFACE:${HEADERS_INSTALLATION_LOCATION}>  # <prefix>/include/mylib
    )

    install(TARGETS ${LIBRARY_NAME} EXPORT ${PROJECT_NAME}Config DESTINATION ${CMAKE_INSTALL_LIBDIR})

	# This makes the project importable from the build directory
	export(TARGETS ${LIBRARY_NAME} FILE ${PROJECT_NAME}Config.cmake)

endfunction()

file(GLOB_RECURSE INCLUDE_FILES *.h)
install(FILES ${INCLUDE_FILES} DESTINATION ${HEADERS_INSTALLATION_LOCATION})

add_subdirectory(common)
add_subdirectory(model)
add_subdirectory(property)
add_subdirectory(qmlmodel)

if(FACELIFT_ENABLE_IPC)
    add_subdirectory(ipc)
endif()

add_subdirectory(dummy)

install(EXPORT ${PROJECT_NAME}Config DESTINATION ${CMAKE_CONFIG_INSTALLATION_PATH})
