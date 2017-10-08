
facelift_add_package(MyPackageGenerated facelift.example.mypackage ${CMAKE_CURRENT_SOURCE_DIR}/interface)

add_definitions(-DQML_MODEL_LOCATION=${CMAKE_CURRENT_SOURCE_DIR})

#facelift_set_qml_implementation_path(MyPackageGenerated "${CMAKE_CURRENT_SOURCE_DIR}/models/qml")

facelift_add_qml_plugin(MyPackagePlugin
    URI facelift.example.mypackage
    SOURCES_GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    HEADERS_GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    LINK_LIBRARIES MyPackageGenerated
)

add_example_script(mypackage ui/MyApp.qml)

if(FACELIFT_ENABLE_IPC)
	add_example_script(mypackage-ipcclient ui/ipc/MyAppIPCClient.qml)
	add_example_script(mypackage-ipcserver ui/ipc/MyAppIPCServer.qml)
endif()
