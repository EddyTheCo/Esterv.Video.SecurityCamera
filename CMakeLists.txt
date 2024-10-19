cmake_minimum_required(VERSION 3.10)

# Project name and version
project(BackgroundSubtractorExample VERSION 1.0)

# Find OpenCV package
find_package(OpenCV REQUIRED)

find_package(Boost REQUIRED COMPONENTS system log)

# Add the executable (main.cpp)
add_executable(BackgroundSubtractor src/main.cpp  src/tcp_server.cpp src/tcp_server.hpp src/camera_service.hpp src/camera_service.cpp)

# Link OpenCV libraries to the executable
target_link_libraries(BackgroundSubtractor PRIVATE ${OpenCV_LIBS})

target_link_libraries(BackgroundSubtractor PRIVATE Boost::system Boost::log)

# Set C++ Standard
set_property(TARGET BackgroundSubtractor PROPERTY CXX_STANDARD 17)

