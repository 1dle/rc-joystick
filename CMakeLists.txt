cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(rc-joystick)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(pigpio REQUIRED)
include_directories(${pigpio_INCLUDE_DIRS})

include_directories(libs/joystick)
add_subdirectory(libs)

add_executable(rc-joystick src/main.cpp src/RemoteRC.cpp src/RemoteRC.h src/Controller.cpp src/Controller.h)

target_link_libraries(rc-joystick PUBLIC joystick)
target_link_libraries(rc-joystick LINK_PRIVATE ${pigpiod_if2_LIBRARY})