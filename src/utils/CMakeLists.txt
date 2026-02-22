add_library(linuxeye_utils STATIC
    fileutils.cpp
    sizeformatter.cpp
    hwutils.cpp
)

target_include_directories(linuxeye_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(linuxeye_utils PUBLIC Qt6::Core)
