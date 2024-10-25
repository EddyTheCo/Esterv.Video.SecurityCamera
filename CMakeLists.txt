cmake_minimum_required(VERSION 3.10)

# Project name and version
project(SecurityCamera VERSION 1.0)

# Find OpenCV package
find_package(OpenCV REQUIRED)

find_package(Boost REQUIRED COMPONENTS system log)

add_executable(server src/server.cpp  src/tcp_server.cpp src/tcp_server.hpp src/camera_service.hpp src/camera_service.cpp)

add_executable(client src/client.cpp)


# Link OpenCV libraries to the executable
target_link_libraries(server PRIVATE ${OpenCV_LIBS} Boost::system Boost::log)

target_link_libraries(client PRIVATE ${OpenCV_LIBS} Boost::system Boost::log)


set_property(TARGET server PROPERTY CXX_STANDARD 17)

