
facelift_add_interface(MyPackageInterface
    INTERFACE_DEFINITION_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/interface
)

add_definitions(-DQML_MODEL_LOCATION=${CMAKE_CURRENT_SOURCE_DIR})

facelift_add_qml_plugin(MyPackagePlugin
    URI facelift.example.mypackage
    SOURCES_GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    HEADERS_GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    LINK_LIBRARIES MyPackageInterface
)

add_example_script(mypackage ui/MyApp.qml)
add_example_script(mypackage-ipcclient-async ui/ipc/MyAppAsyncIPCClient.qml)
add_example_script(mypackage-ipcclient ui/ipc/MyAppIPCClient.qml)
add_example_script(mypackage-ipcserver ui/ipc/MyAppIPCServer.qml)
add_example_script(mypackage-inprocess ui/ipc/MyAppInProcess.qml)
