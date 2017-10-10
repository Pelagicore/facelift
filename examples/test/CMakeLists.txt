file(GLOB_RECURSE CPP_SOURCE_FILES)
file(GLOB_RECURSE HEADER_FILES )

facelift_add_qml_plugin(TestPlugin 
   URI facelift.test
   SOURCES_GLOB_RECURSE  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
   HEADERS_GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/*.h
   LINK_LIBRARIES TestGenerated
)

add_example_script(test ui/TestUI.qml)
