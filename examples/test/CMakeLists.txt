#Uncomment to use QML implementation:
#add_definitions(-DQML_MODEL_LOCATION=${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

facelift_add_qml_plugin(TestPlugin 
   URI facelift.test
   SOURCES_GLOB_RECURSE  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
   HEADERS_GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/*.h
   LINK_LIBRARIES TestInterface
)

add_example_script(test ui/TestUI.qml)
