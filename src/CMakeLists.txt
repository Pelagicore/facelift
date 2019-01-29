
add_subdirectory(common)
add_subdirectory(model)
add_subdirectory(ipc)

if(FACELIFT_ENABLE_DESKTOP_DEV_TOOLS)
    add_subdirectory(desktop-dev-tools)
endif()

add_subdirectory(plugin)
