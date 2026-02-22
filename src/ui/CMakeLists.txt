add_library(linuxeye_ui STATIC
    mainwindow.cpp
    sidebar.cpp
    overviewpanel.cpp
    cpupanel.cpp
    memorypanel.cpp
    ospanel.cpp
    storagepanel.cpp
    networkpanel.cpp
    gpupanel.cpp
    boardpanel.cpp
    sensorpanel.cpp
)

target_include_directories(linuxeye_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(linuxeye_ui PUBLIC Qt6::Core Qt6::Widgets linuxeye_core)
