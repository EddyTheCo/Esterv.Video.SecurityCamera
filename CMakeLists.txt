cmake_minimum_required(VERSION 3.10)

# Project name and version
project(BackgroundSubtractorExample VERSION 1.0)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Add the executable (main.cpp)
add_executable(BackgroundSubtractor main.cpp)

# Link OpenCV libraries to the executable
target_link_libraries(BackgroundSubtractor ${OpenCV_LIBS})

# Set C++ Standard
set_property(TARGET BackgroundSubtractor PROPERTY CXX_STANDARD 11)

