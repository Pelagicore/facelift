
set(AUTO_UNITY_BUILD ON)

add_subdirectory(common)
add_subdirectory(model)
add_subdirectory(property)
add_subdirectory(qmlmodel)

if(FACELIFT_ENABLE_IPC)
    add_subdirectory(ipc)
endif()

add_subdirectory(dummy)
