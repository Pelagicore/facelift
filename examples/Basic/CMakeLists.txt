
facelift_add_package(MyPackageGenerated facelift.example.mypackage ${CMAKE_CURRENT_SOURCE_DIR}/interface)

add_definitions(-DQML_MODEL_LOCATION=${CMAKE_CURRENT_SOURCE_DIR})

#facelift_set_qml_implementation_path(MyPackageGenerated "${CMAKE_CURRENT_SOURCE_DIR}/models/qml")

file(GLOB_RECURSE CPP_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

facelift_add_qml_plugin(MyPackagePlugin facelift.example.mypackage 1 0 "${CPP_SOURCE_FILES}" "${HEADER_FILES}")
target_link_libraries(MyPackagePlugin MyPackageGenerated)
target_include_directories(MyPackagePlugin INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/MyPackage)

add_example_script(mypackage ui/MyApp.qml)

if(FACELIFT_ENABLE_IPC)
	add_example_script(mypackage-ipcclient ui/ipc/MyAppIPCClient.qml)
	add_example_script(mypackage-ipcserver ui/ipc/MyAppIPCServer.qml)
endif()
