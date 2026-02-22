add_subdirectory(utils)
add_subdirectory(collectors)
add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(helper)

add_executable(linuxeye main.cpp)

target_link_libraries(linuxeye
    PRIVATE
    linuxeye_ui
    linuxeye_core
    linuxeye_collectors
    linuxeye_utils
    Qt6::Core
    Qt6::Widgets
)

# Install binary
install(TARGETS linuxeye
    RUNTIME DESTINATION bin
)
