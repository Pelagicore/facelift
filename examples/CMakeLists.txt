
add_subdirectory(AddressBook)
add_subdirectory(Tuner)
add_subdirectory(Mediaplayer)
add_subdirectory(Basic)

if(FACELIFT_ENABLE_IPC)
add_subdirectory(IPC)
endif()
