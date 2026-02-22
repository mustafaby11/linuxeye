add_executable(linuxeye-helper main.cpp)

target_compile_features(linuxeye-helper PRIVATE cxx_std_17)

install(TARGETS linuxeye-helper
    RUNTIME DESTINATION bin
)
