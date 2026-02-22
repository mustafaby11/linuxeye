add_library(linuxeye_collectors STATIC
    cpucollector.cpp
    memorycollector.cpp
    oscollector.cpp
    storagecollector.cpp
    networkcollector.cpp
    gpucollector.cpp
    boardcollector.cpp
    sensorcollector.cpp
    privilegedcollector.h
    privilegedcollector.cpp
)

target_include_directories(linuxeye_collectors PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(linuxeye_collectors PUBLIC Qt6::Core Qt6::Network linuxeye_utils)
