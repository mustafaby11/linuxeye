add_library(linuxeye_core STATIC
    application.cpp
    systeminfo.cpp
)

target_include_directories(linuxeye_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(linuxeye_core PUBLIC Qt6::Core Qt6::Widgets linuxeye_collectors linuxeye_utils)
