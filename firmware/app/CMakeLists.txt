# CMakeLists.txt for Whoop Alternative Firmware

cmake_minimum_required(VERSION 3.20.0)

# Find Zephyr and include boilerplate
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(whoop_alternative)

# Add source files
target_sources(app PRIVATE
    src/main.c
    src/app_states.c
)

# Add include directories
target_include_directories(app PRIVATE
    src/
    ../drivers/
    ../modules/
    ../ble/
    ../storage/
    ../power/
)

# Add driver libraries (will be created later)
# target_link_libraries(app PRIVATE
#     ppg_driver
#     imu_driver
#     temp_driver
#     flash_driver
#     fuel_driver
# )

# Add module libraries (will be created later)
# target_link_libraries(app PRIVATE
#     ppg_pipeline
#     hrv_module
#     resp_module
#     sleep_module
#     strain_module
#     health_trends
# )
