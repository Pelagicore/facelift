configure_file(test-driver.sh.in test-driver.sh @ONLY)


if (CMAKE_CONFIGURATION_TYPES)
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure
                      --build-config "$<CONFIGURATION>")
else()
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure)
endif()


add_subdirectory(manual)

add_subdirectory(combined)
add_test(NAME combined-local-cpp COMMAND test-driver.sh combined/tst_combined_local.qml cpp)
add_test(NAME combined-local-qml COMMAND test-driver.sh combined/tst_combined_local.qml qml)
add_test(NAME combined-ipc-cpp COMMAND test-driver.sh combined/tst_combined_ipc.qml cpp combined/combined-server.qml)
add_test(NAME combined-ipc-qml COMMAND test-driver.sh combined/tst_combined_ipc.qml qml combined/combined-server.qml)
add_test(NAME combined-inprocess-cpp COMMAND test-driver.sh combined/tst_combined_inprocess.qml cpp)
add_test(NAME combined-inprocess-qml COMMAND test-driver.sh combined/tst_combined_inprocess.qml qml)



add_subdirectory(asynctests)
add_test(NAME asynctests-local-cpp COMMAND test-driver.sh asynctests/tst_asynctests_local.qml cpp)
add_test(NAME asynctests-local-qml COMMAND test-driver.sh asynctests/tst_asynctests_local.qml qml)
add_test(NAME asynctests-ipc-cpp COMMAND test-driver.sh asynctests/tst_asynctests_ipc.qml cpp asynctests/asynctests-server.qml)
add_test(NAME asynctests-ipc-qml COMMAND test-driver.sh asynctests/tst_asynctests_ipc.qml qml asynctests/asynctests-server.qml)
add_test(NAME asynctests-inprocess-cpp COMMAND test-driver.sh asynctests/tst_asynctests_inprocess.qml cpp)
add_test(NAME asynctests-inprocess-qml COMMAND test-driver.sh asynctests/tst_asynctests_inprocess.qml qml)


