cmake_minimum_required(VERSION 3.20)
project(linuxeye VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# Include subdirectory
add_subdirectory(src)

# Install Polkit policy
install(FILES data/io.linuxeye.collect.policy
    DESTINATION share/polkit-1/actions
)
